#include "Application.h"
#include "PluginManager.h"
#include "World.h"
#include "GraphicsHelpers.h"

HA_SUPPRESS_WARNINGS

#include <bgfx/c99/platform.h>

#include <GLFW/glfw3.h>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#else // EMSCRIPTEN

#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#elif BX_PLATFORM_OSX
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#elif BX_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif // platforms

#if !BX_PLATFORM_WINDOWS
#include <unistd.h> // for chdir()
#endif              // not windows

#include <GLFW/glfw3native.h>
#endif // EMSCRIPTEN

HA_SUPPRESS_WARNINGS_END

// =================================================================================================
// == IMGUI ========================================================================================
// =================================================================================================

static bgfx_vertex_decl    ivd;
static bgfx_texture_handle imguiFontTexture;
static bgfx_uniform_handle imguiFontUniform;
static bgfx_program_handle imguiProgram;
static void                imguiRender(ImDrawData* drawData);
static void                imguiShutdown();
static cstr                imguiGetClipboardText(void* userData);
static void                imguiSetClipboardText(void* userData, cstr text);

static void imguiInit() {
    unsigned char* data;
    int            width, height;
    ImGuiIO&       io = ImGui::GetIO();

    // Setup vertex declaration
    bgfx_vertex_decl_begin(&ivd, BGFX_RENDERER_TYPE_COUNT);
    bgfx_vertex_decl_add(&ivd, BGFX_ATTRIB_POSITION, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
    bgfx_vertex_decl_add(&ivd, BGFX_ATTRIB_TEXCOORD0, 2, BGFX_ATTRIB_TYPE_FLOAT, false, false);
    bgfx_vertex_decl_add(&ivd, BGFX_ATTRIB_COLOR0, 4, BGFX_ATTRIB_TYPE_UINT8, true, false);
    bgfx_vertex_decl_end(&ivd);

    // Create font
    io.Fonts->AddFontDefault();
    io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);
    imguiFontTexture = bgfx_create_texture_2d((uint16)width, (uint16)height, false, 1,
                                              BGFX_TEXTURE_FORMAT_BGRA8, 0,
                                              bgfx_copy(data, width * height * 4));
    imguiFontUniform = bgfx_create_uniform("s_tex", BGFX_UNIFORM_TYPE_INT1, 1);

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
        bgfx_transient_vertex_buffer tvb;
        bgfx_transient_index_buffer  tib;

        const ImDrawList* drawList    = drawData->CmdLists[ii];
        uint32            numVertices = (uint32)drawList->VtxBuffer.size();
        uint32            numIndices  = (uint32)drawList->IdxBuffer.size();

        if(!bgfx_get_avail_transient_vertex_buffer(numVertices, &ivd) ||
           !bgfx_get_avail_transient_index_buffer(numIndices)) {
            break;
        }

        bgfx_alloc_transient_vertex_buffer(&tvb, numVertices, &ivd);
        bgfx_alloc_transient_index_buffer(&tib, numIndices);

        HA_CLANG_SUPPRESS_WARNING("-Wcast-align")
        ImDrawVert* verts = (ImDrawVert*)tvb.data;
        memcpy(verts, drawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert));

        ImDrawIdx* indices = (ImDrawIdx*)tib.data;
        memcpy(indices, drawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));
        HA_CLANG_SUPPRESS_WARNING_END

        uint32 offset = 0;
        for(const ImDrawCmd *cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end();
            cmd != cmdEnd; ++cmd) {
            if(cmd->UserCallback) {
                cmd->UserCallback(drawList, cmd);
            } else if(0 != cmd->ElemCount) {
                uint64_t state = BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_MSAA;
                bgfx_texture_handle th = imguiFontTexture;
                if(cmd->TextureId != nullptr) {
                    union
                    {
                        ImTextureID ptr;
                        struct
                        {
                            uint16              flags;
                            bgfx_texture_handle handle;
                        } s;
                    } texture = {cmd->TextureId};
                    HA_SUPPRESS_WARNINGS
                    state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                                                   BGFX_STATE_BLEND_INV_SRC_ALPHA);
                    HA_SUPPRESS_WARNINGS_END
                    th = texture.s.handle;
                } else {
                    HA_SUPPRESS_WARNINGS
                    state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                                                   BGFX_STATE_BLEND_INV_SRC_ALPHA);
                    HA_SUPPRESS_WARNINGS_END
                }
                const uint16 xx = uint16(Utils::Max(cmd->ClipRect.x, 0.0f));
                const uint16 yy = uint16(Utils::Max(cmd->ClipRect.y, 0.0f));
                bgfx_set_scissor(xx, yy, uint16(Utils::Min(cmd->ClipRect.z, 65535.0f) - xx),
                                 uint16(Utils::Min(cmd->ClipRect.w, 65535.0f) - yy));
                bgfx_set_state(state, 0);
                bgfx_set_texture(0, imguiFontUniform, th, UINT32_MAX);
                bgfx_set_transient_vertex_buffer(0, &tvb, 0, numVertices);
                bgfx_set_transient_index_buffer(&tib, offset, cmd->ElemCount);
                bgfx_submit(0, imguiProgram, 0, false);
            }

            offset += cmd->ElemCount;
        }
    }
}

static void imguiShutdown() {
    bgfx_destroy_uniform(imguiFontUniform);
    bgfx_destroy_texture(imguiFontTexture);
    bgfx_destroy_program(imguiProgram);
    ImGui::Shutdown();
}

static cstr imguiGetClipboardText(void* userData) {
    return glfwGetClipboardString((GLFWwindow*)userData);
}

static void imguiSetClipboardText(void* userData, cstr text) {
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

// TODO: use a smarter allocator - the important methods here are for the mixin data
class global_mixin_allocator : public dynamix::global_allocator
{
    char* alloc_mixin_data(size_t count) override { return new char[count * mixin_data_size]; }
    void  dealloc_mixin_data(char* ptr) override { delete[] ptr; }

    void alloc_mixin(size_t mixin_size, size_t mixin_alignment, char*& out_buffer,
                     size_t& out_mixin_offset) override {
        const size_t size = calculate_mem_size_for_mixin(mixin_size, mixin_alignment);
        out_buffer        = new char[size];
        out_mixin_offset  = calculate_mixin_offset(out_buffer, mixin_alignment);
    }
    void dealloc_mixin(char* ptr) override { delete[] ptr; }
};

HA_SINGLETON_INSTANCE(Application);

void Application::addInputEventListener(InputEventListener* in) {
    hassert(std::find(m_inputEventListeners.begin(), m_inputEventListeners.end(), in) ==
            m_inputEventListeners.end());
    m_inputEventListeners.push_back(in);
}
void Application::removeInputEventListener(InputEventListener* in) {
    auto it = std::find(m_inputEventListeners.begin(), m_inputEventListeners.end(), in);
    hassert(it != m_inputEventListeners.end());
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
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif // EMSCRIPTEN

    // Create a window
    m_window = glfwCreateWindow(width(), height(), "game", nullptr, nullptr);
    if(!m_window) {
        glfwTerminate();
        return -1;
    }

#ifndef EMSCRIPTEN
    // Simulating fullscreen the way bgfx does in its example entry - by placing the window in 0,0
    int monitor_pos_x, monitor_pos_y;
    glfwGetMonitorPos(monitor, &monitor_pos_x, &monitor_pos_y);
    glfwSetWindowPos(m_window, monitor_pos_x, monitor_pos_y);
    //glfwSetWindowMonitor(m_window, monitor, 0, 0, width(), height(), mode->refreshRate);
#endif // EMSCRIPTEN

    // Setup input callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCharCallback(m_window, charCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    glfwSetCursorPos(m_window, width() / 2, height() / 2);

    // Setup bgfx
    bgfx_platform_data pd;
    memset(&pd, 0, sizeof(pd));
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
    pd.ndt = glfwGetX11Display();
    pd.nwh = (void*)glfwGetX11Window(m_window);
#elif BX_PLATFORM_OSX
    pd.nwh = glfwGetCocoaWindow(m_window);
#elif BX_PLATFORM_WINDOWS
    pd.nwh = glfwGetWin32Window(m_window);
#endif // BX_PLATFORM_

    bgfx_set_platform_data(&pd);
    bgfx_init(BGFX_RENDERER_TYPE_OPENGL, BGFX_PCI_ID_NONE, 0, nullptr, nullptr);

    // Setup ImGui
    imguiInit();

    // Initialize the application
    reset();
    bgfx_set_debug(BGFX_DEBUG_TEXT);

    // introduce this scope in order to control the lifetimes of managers
    {
        // resource managers should be created first and destroyed last - all
        // objects should be destroyed so the refcounts to the resources are 0
        MeshMan   meshMan;
        ShaderMan shaderMan;
        GeomMan   geomMan;

        ObjectManager objectMan;

        World world;

#ifdef EMSCRIPTEN
        emscripten_set_main_loop([]() { Application::get().update(); }, 0, 1);
#else  // EMSCRIPTEN
        while(!glfwWindowShouldClose(m_window))
            update();
#endif // EMSCRIPTEN
    }

    imguiShutdown();
    bgfx_shutdown();
    glfwTerminate();
    return tests_res;
}

void Application::processEvents() {
    ImGuiIO& io = ImGui::GetIO();

    for(size_t i = 0; i < m_inputs.size(); ++i)
        for(auto& curr : m_inputEventListeners)
            if((m_inputs[i].type == InputEvent::KEY && !io.WantCaptureKeyboard) ||
               (m_inputs[i].type == InputEvent::MOTION && !io.WantCaptureMouse) ||
               (m_inputs[i].type == InputEvent::BUTTON && !io.WantCaptureMouse) ||
               (m_inputs[i].type == InputEvent::SCROLL && !io.WantCaptureMouse))
                curr->process_event(m_inputs[i]);

    m_inputs.clear();
}

void Application::update() {
#ifdef HA_WITH_PLUGINS
    // reload plugins
    PluginManager::get().update();
#endif // HA_WITH_PLUGINS

    m_time     = float(glfwGetTime());
    m_dt       = m_time - m_lastTime;
    m_lastTime = m_time;

    // poll for events - also dispatches to imgui
    glfwPollEvents();
    imguiEvents(m_dt);

    // imgui
    ImGui::NewFrame();

    // send input events to the rest of the app
    processEvents();

    // update game stuff
    World::get().update();

    // render
    ImGui::Render();
    bgfx_frame(false);

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
    bgfx_reset(m_width, m_height, m_reset);
    bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x880088ff, 1.0f, 0);
    bgfx_set_view_rect(0, 0, 0, uint16(width()), uint16(height()));
}
