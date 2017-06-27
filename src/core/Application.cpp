#include "Application.h"
#include "PluginManager.h"
#include "ObjectManager.h"
#include "utils/utils.h"
#include "core/GraphicsHelpers.h"

#include "core/messages/messages.h"

HA_SUPPRESS_WARNINGS

#include <bgfx/platform.h>

#include <GLFW/glfw3.h>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#else // EMSCRIPTEN
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#endif // _WIN32
#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#endif // __APPLE__
#include <GLFW/glfw3native.h>
#endif // EMSCRIPTEN

HA_SUPPRESS_WARNINGS_END

// =================================================================================================
// == IMGUI ========================================================================================
// =================================================================================================

static bgfx::VertexDecl    imguiVertexDecl;
static bgfx::TextureHandle imguiFontTexture;
static bgfx::UniformHandle imguiFontUniform;
static bgfx::ProgramHandle imguiProgram;
static void imguiRender(ImDrawData* drawData);
static void        imguiShutdown();
static const char* imguiGetClipboardText(void* userData);
static void imguiSetClipboardText(void* userData, const char* text);

static void imguiInit() {
    unsigned char* data;
    int            width, height;
    ImGuiIO&       io = ImGui::GetIO();

    // Setup vertex declaration
    imguiVertexDecl.begin()
            .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
            .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
            .end();

    // Create font
    io.Fonts->AddFontDefault();
    io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);
    imguiFontTexture = bgfx::createTexture2D((uint16_t)width, (uint16_t)height, false, 1,
                                             bgfx::TextureFormat::BGRA8, 0,
                                             bgfx::copy(data, width * height * 4));
    imguiFontUniform = bgfx::createUniform("s_tex", bgfx::UniformType::Int1);

    // Create shader program
    imguiProgram = loadProgram("ocornut_imgui_vs", "ocornut_imgui_fs");

    // Setup render callback
    io.RenderDrawListsFn = imguiRender;

    // Key mapping
    io.KeyMap[ImGuiKey_Tab]        = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]  = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]    = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow]  = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp]     = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown]   = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home]       = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End]        = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete]     = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace]  = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter]      = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape]     = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A]          = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C]          = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V]          = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X]          = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y]          = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z]          = GLFW_KEY_Z;
    io.SetClipboardTextFn          = imguiSetClipboardText;
    io.GetClipboardTextFn          = imguiGetClipboardText;
}

static void imguiRender(ImDrawData* drawData) {
    for(int ii = 0, num = drawData->CmdListsCount; ii < num; ++ii) {
        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer  tib;

        const ImDrawList* drawList    = drawData->CmdLists[ii];
        uint32            numVertices = (uint32)drawList->VtxBuffer.size();
        uint32            numIndices  = (uint32)drawList->IdxBuffer.size();

        if(!bgfx::getAvailTransientVertexBuffer(numVertices, imguiVertexDecl) ||
           !bgfx::getAvailTransientIndexBuffer(numIndices)) {
            break;
        }

        bgfx::allocTransientVertexBuffer(&tvb, numVertices, imguiVertexDecl);
        bgfx::allocTransientIndexBuffer(&tib, numIndices);

        ImDrawVert* verts = (ImDrawVert*)tvb.data;
        memcpy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert));

        ImDrawIdx* indices = (ImDrawIdx*)tib.data;
        memcpy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));

        uint32 offset = 0;
        for(const ImDrawCmd *cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end();
            cmd != cmdEnd; ++cmd) {
            if(cmd->UserCallback) {
                cmd->UserCallback(drawList, cmd);
            } else if(0 != cmd->ElemCount) {
                uint64_t state = BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_MSAA;
                bgfx::TextureHandle th = imguiFontTexture;
                if(cmd->TextureId != nullptr) {
                    union
                    {
                        ImTextureID ptr;
                        struct
                        {
                            uint16_t            flags;
                            bgfx::TextureHandle handle;
                        } s;
                    } texture = {cmd->TextureId};
                    state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                                                   BGFX_STATE_BLEND_INV_SRC_ALPHA);
                    th = texture.s.handle;
                } else {
                    state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                                                   BGFX_STATE_BLEND_INV_SRC_ALPHA);
                }
                const uint16_t xx = uint16_t(Utils::Max(cmd->ClipRect.x, 0.0f));
                const uint16_t yy = uint16_t(Utils::Max(cmd->ClipRect.y, 0.0f));
                bgfx::setScissor(xx, yy, uint16_t(Utils::Min(cmd->ClipRect.z, 65535.0f) - xx),
                                 uint16_t(Utils::Min(cmd->ClipRect.w, 65535.0f) - yy));
                bgfx::setState(state);
                bgfx::setTexture(0, imguiFontUniform, th);
                bgfx::setVertexBuffer(0, &tvb, 0, numVertices);
                bgfx::setIndexBuffer(&tib, offset, cmd->ElemCount);
                bgfx::submit(0, imguiProgram);
            }

            offset += cmd->ElemCount;
        }
    }
}

static void imguiShutdown() {
    bgfx::destroyUniform(imguiFontUniform);
    bgfx::destroyTexture(imguiFontTexture);
    bgfx::destroyProgram(imguiProgram);
    ImGui::Shutdown();
}

static const char* imguiGetClipboardText(void* userData) {
    return glfwGetClipboardString((GLFWwindow*)userData);
}

static void imguiSetClipboardText(void* userData, const char* text) {
    glfwSetClipboardString((GLFWwindow*)userData, text);
}

// =================================================================================================
// == APPLICATION INPUT ============================================================================
// =================================================================================================

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS && button >= 0 && button < 3)
        app->m_mousePressed[button] = true;

    InputEvent ev;
    ev.button = {InputEvent::BUTTON, button, action};
    Application::get().addInputEvent(ev);
}

void Application::scrollCallback(GLFWwindow* window, double, double yoffset) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
#ifdef EMSCRIPTEN
    yoffset *=
            -0.01; // fix emscripten/glfw bug - probably related to this: https://github.com/kripken/emscripten/issues/3171
#endif             // EMSCRIPTEN
    app->m_mouseWheel += float(yoffset);

    InputEvent ev;
    ev.scroll = {InputEvent::SCROLL, yoffset};
    Application::get().addInputEvent(ev);
}

void Application::keyCallback(GLFWwindow*, int key, int, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if(action == GLFW_PRESS)
        io.KeysDown[key] = true;

    if(action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    io.KeyCtrl  = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt   = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];

    InputEvent ev;
    ev.key = {InputEvent::KEY, key, action, mods};
    Application::get().addInputEvent(ev);
}

void Application::charCallback(GLFWwindow*, unsigned int c) {
    ImGuiIO& io = ImGui::GetIO();
    if(c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

void Application::cursorPosCallback(GLFWwindow*, double x, double y) {
    static double last_x = 0.0;
    static double last_y = 0.0;
    InputEvent    ev;
    ev.motion = {InputEvent::MOTION, x, y, x - last_x, y - last_y};
    last_x    = x;
    last_y    = y;

    Application::get().addInputEvent(ev);
}

void imguiEvents(float dt) {
    Application& app = Application::get();
    ImGuiIO&     io  = ImGui::GetIO();
    io.DeltaTime     = dt;
    int w, h;
    int displayW, displayH;
    glfwGetWindowSize(app.m_window, &w, &h);
    glfwGetFramebufferSize(app.m_window, &displayW, &displayH);
    io.DisplaySize             = ImVec2(float(w), float(h));
    io.IniFilename             = ""; //"imgui.ini";
    io.DisplayFramebufferScale = ImVec2(float(displayW) / w, float(displayH) / h);
    double mouse_x, mouse_y;
    glfwGetCursorPos(app.m_window, &mouse_x, &mouse_y);
    io.MousePos = ImVec2(float(mouse_x), float(mouse_y));
    for(int i = 0; i < 3; i++) {
        io.MouseDown[i]       = app.m_mousePressed[i] || glfwGetMouseButton(app.m_window, i) != 0;
        app.m_mousePressed[i] = false;
    }
    io.MouseWheel    = app.m_mouseWheel;
    app.m_mouseWheel = 0.0f;
    glfwSetInputMode(app.m_window, GLFW_CURSOR,
                     io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    io.ClipboardUserData = app.m_window;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(app.m_window);
#endif // _WIN32
}

// =================================================================================================
// == APPLICATION IMPLEMENTATION ===================================================================
// =================================================================================================

HA_SCOPED_SINGLETON_IMPLEMENT(Application);

// TODO: use a smarter allocator - the important methods here are for the mixin data
class global_mixin_allocator : public dynamix::global_allocator
{
    char* alloc_mixin_data(size_t count) override { return new char[count * mixin_data_size]; }
    void dealloc_mixin_data(char* ptr) override { delete[] ptr; }

    void alloc_mixin(size_t mixin_size, size_t mixin_alignment, char*& out_buffer,
                     size_t& out_mixin_offset) override {
        const size_t size = calculate_mem_size_for_mixin(mixin_size, mixin_alignment);
        out_buffer        = new char[size];
        out_mixin_offset  = calculate_mixin_offset(out_buffer, mixin_alignment);
    }
    void dealloc_mixin(char* ptr) override { delete[] ptr; }
};

void Application::addInputEventListener(int in) { m_inputEventListeners.push_back(in); }
void Application::removeInputEventListener(int in) {
    auto it = std::find(m_inputEventListeners.begin(), m_inputEventListeners.end(), in);
    PPK_ASSERT(it != m_inputEventListeners.end());
    m_inputEventListeners.erase(it);
}

int Application::run(int argc, char** argv) {
#ifndef EMSCRIPTEN
#ifdef _WIN32
#define HA_SET_DATA_CWD SetCurrentDirectory
#else // _WIN32
#define HA_SET_DATA_CWD chdir
#endif // _WIN32
    // set cwd to data folder - this is done for emscripten with the --preload-file flag
    HA_SET_DATA_CWD((Utils::getPathToExe() + "../../../data").c_str());
#endif // EMSCRIPTEN

#ifdef HA_WITH_PLUGINS
    // load plugins first so tests in them get executed as well
    PluginManager pluginManager;
    pluginManager.init();
#endif // HA_WITH_PLUGINS

    // run tests
    doctest::Context context(argc, argv);
    int              tests_res = context.run();
    if(context.shouldExit())
        return tests_res;

    // setup global dynamix allocator before any objects are created
    global_mixin_allocator alloc;
    dynamix::set_global_allocator(&alloc);

    // Initialize glfw
    if(!glfwInit())
        return -1;

#ifndef EMSCRIPTEN
    int           num_monitors = 0;
    GLFWmonitor** monitors     = glfwGetMonitors(&num_monitors);
    //GLFWmonitor*     monitor = glfwGetPrimaryMonitor();
    GLFWmonitor*       monitor = monitors[num_monitors - 1];
    const GLFWvidmode* mode    = glfwGetVideoMode(monitor);
    m_width                    = mode->width;
    m_height                   = mode->height;
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
#endif // EMSCRIPTEN

    // Create a window
    m_window = glfwCreateWindow(getWidth(), getHeight(), "game", nullptr, nullptr);
    if(!m_window) {
        glfwTerminate();
        return -1;
    }

#ifndef EMSCRIPTEN
    // Simulating fullscreen the way bgfx does... by placing the window in 0,0
    glfwSetWindowMonitor(m_window, monitor, 0, 0, getWidth(), getHeight(), mode->refreshRate);
#endif // EMSCRIPTEN

    // Setup input callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCharCallback(m_window, charCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);
    
    glfwSetCursorPos(m_window, getWidth() / 2, getHeight() / 2);

    // Setup bgfx
    bgfx::PlatformData platformData;
    memset(&platformData, 0, sizeof(platformData));
#ifdef _WIN32
    platformData.nwh = glfwGetWin32Window(m_window);
#endif // _WIN32
#ifdef __APPLE__
    platformData.nwh = glfwGetCocoaWindow(m_window);
#endif // __APPLE__
    bgfx::setPlatformData(platformData);
    bgfx::init(bgfx::RendererType::OpenGL); // can also not specify opengl at all

    // Setup ImGui
    imguiInit();

    // Initialize the application
    reset();

    // create game
    ObjectManager objectManager;
    objectManager.init();

#ifdef EMSCRIPTEN
    emscripten_set_main_loop([]() { Application::get().update(); }, 0, 1);
#else  // EMSCRIPTEN
    // Loop until the user closes the window
    while(!glfwWindowShouldClose(m_window))
        update();
#endif // EMSCRIPTEN

    // Shutdown in reverse order of initialization
    objectManager.shutdown();
    imguiShutdown();
    bgfx::shutdown();
    glfwTerminate();
    return tests_res;
}

void Application::processEvents() {
    glfwPollEvents();

    for(size_t i = 0; i < m_inputs.size(); ++i)
        for(auto& curr : m_inputEventListeners)
            process_event(ObjectManager::get().get_object(curr), m_inputs[i]);

    m_inputs.clear();

    imguiEvents(m_dt);
}

void Application::update() {
#ifdef HA_WITH_PLUGINS
    // reload plugins
    PluginManager::get().update();
#endif // HA_WITH_PLUGINS

    m_time     = float(glfwGetTime());
    m_dt       = m_time - m_lastTime;
    m_lastTime = m_time;

    // events
    processEvents();

    // imgui
    ImGui::NewFrame();

    // update game stuff
    ObjectManager::get().update();

    // render
    ImGui::Render();
    bgfx::frame();

    // check if should close
    if(glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, GL_TRUE);

    // handle resizing
    int w, h;
    glfwGetWindowSize(m_window, &w, &h);
    if(uint32(w) != m_width || uint32(h) != m_height) {
        m_width  = w;
        m_height = h;
        reset(m_reset);
    }
}

void Application::reset(uint32 flags) {
    m_reset = flags;
    bgfx::reset(m_width, m_height, m_reset);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xff00ffff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t(getHeight()));
}
