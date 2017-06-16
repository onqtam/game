#include "Application.h"
#include "PluginManager.h"
#include "ObjectManager.h"
#include "utils/utils.h"
#include "core/GraphicsHelpers.h"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <bx/fpumath.h>

#include <GLFW/glfw3.h>
#include <imgui.h>

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
    imguiProgram = loadProgram("vs_ocornut_imgui", "fs_ocornut_imgui");

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
        uint32_t          numVertices = (uint32_t)drawList->VtxBuffer.size();
        uint32_t          numIndices  = (uint32_t)drawList->IdxBuffer.size();

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

        uint32_t offset = 0;
        for(const ImDrawCmd *cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end();
            cmd != cmdEnd; ++cmd) {
            if(cmd->UserCallback) {
                cmd->UserCallback(drawList, cmd);
            } else if(0 != cmd->ElemCount) {
                uint64_t state = BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_MSAA;
                bgfx::TextureHandle th = imguiFontTexture;
                if(cmd->TextureId != NULL) {
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
                const uint16_t xx = uint16_t(bx::fmax(cmd->ClipRect.x, 0.0f));
                const uint16_t yy = uint16_t(bx::fmax(cmd->ClipRect.y, 0.0f));
                bgfx::setScissor(xx, yy, uint16_t(bx::fmin(cmd->ClipRect.z, 65535.0f) - xx),
                                 uint16_t(bx::fmin(cmd->ClipRect.w, 65535.0f) - yy));
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

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS && button >= 0 && button < 3)
        app->mMousePressed[button] = true;
}

void Application::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    app->mMouseWheel += (float)yoffset;
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
}

void Application::charCallback(GLFWwindow*, unsigned int c) {
    ImGuiIO& io = ImGui::GetIO();
    if(c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

void Application::cursorPosCallback(GLFWwindow*, double, double) {}

void imguiEvents(float dt) {
    Application& app = Application::get();
    ImGuiIO&     io  = ImGui::GetIO();
    io.DeltaTime     = dt;
    int w, h;
    int displayW, displayH;
    glfwGetWindowSize(app.mWindow, &w, &h);
    glfwGetFramebufferSize(app.mWindow, &displayW, &displayH);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale =
            ImVec2(w > 0 ? ((float)displayW / w) : 0, h > 0 ? ((float)displayH / h) : 0);
    //if(glfwGetWindowAttrib(app.mWindow, GLFW_FOCUSED)) {
    double mouse_x, mouse_y;
    glfwGetCursorPos(app.mWindow, &mouse_x, &mouse_y);
    io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
    //} else {
    //    io.MousePos = ImVec2(-1, -1);
    //}
    for(int i = 0; i < 3; i++) {
        io.MouseDown[i]      = app.mMousePressed[i] || glfwGetMouseButton(app.mWindow, i) != 0;
        app.mMousePressed[i] = false;
    }
    io.MouseWheel   = app.mMouseWheel;
    app.mMouseWheel = 0.0f;
    glfwSetInputMode(app.mWindow, GLFW_CURSOR,
                     io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    io.ClipboardUserData = app.mWindow;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(app.mWindow);
#endif // _WIN32
}

// =================================================================================================
// == APPLICATION IMPLEMENTATION ===================================================================
// =================================================================================================

HARDLY_SCOPED_SINGLETON_IMPLEMENT(Application);

int Application::run(int argc, char** argv) {
#ifndef EMSCRIPTEN
// set cwd to data folder
#ifdef _WIN32
    SetCurrentDirectory((Utils::getPathToExe() + "../../../data").c_str());
#else  // _WIN32
    chdir((Utils::getPathToExe() + "../../../data").c_str());
#endif // _WIN32
    PluginManager pluginManager;
    pluginManager.init();
#endif // EMSCRIPTEN
    ObjectManager objectManager;

    // run tests
    doctest::Context context(argc, argv);
    int              tests_res = context.run();
    if(context.shouldExit())
        return tests_res;

    // Initialize the glfw
    if(!glfwInit())
        return -1;

    // Create a window
    mWindow = glfwCreateWindow(getWidth(), getHeight(), "game", nullptr, nullptr);
    if(!mWindow) {
        glfwTerminate();
        return -1;
    }

    // Setup input callbacks
    glfwSetWindowUserPointer(mWindow, this);
    glfwSetMouseButtonCallback(mWindow, mouseButtonCallback);
    glfwSetScrollCallback(mWindow, scrollCallback);
    glfwSetKeyCallback(mWindow, keyCallback);
    glfwSetCharCallback(mWindow, charCallback);
    glfwSetCursorPosCallback(mWindow, cursorPosCallback);

    // Setup bgfx
    bgfx::PlatformData platformData;
    memset(&platformData, 0, sizeof(platformData));
#ifdef _WIN32
    platformData.nwh = glfwGetWin32Window(mWindow);
#endif // _WIN32
#ifdef __APPLE__
    platformData.nwh = glfwGetCocoaWindow(mWindow);
#endif // __APPLE__
    bgfx::setPlatformData(platformData);
    bgfx::init(bgfx::RendererType::OpenGL); // can also not specify opengl - will choose automatically

    // Setup ImGui
    imguiInit();

    // Initialize the application
    reset();
    objectManager.init();

#ifdef EMSCRIPTEN
    emscripten_set_main_loop([]() { Application::get().update(); }, 0, 1);
#else  // EMSCRIPTEN
    // Loop until the user closes the window
    while(!glfwWindowShouldClose(mWindow))
        update();
#endif // EMSCRIPTEN

    // Shutdown in reverse order of initialization
    imguiShutdown();
    bgfx::shutdown();
    glfwTerminate();
    return tests_res;
}

void Application::update() {
    time     = (float)glfwGetTime();
    dt       = time - lastTime;
    lastTime = time;

    // events
    glfwPollEvents();

    // imgui
    imguiEvents(dt);
    ImGui::NewFrame();

#ifndef EMSCRIPTEN
    // reload plugins
    PluginManager::get().update();
#endif // EMSCRIPTEN

    // update game stuff
    ObjectManager::get().update();

    // render
    ImGui::Render();
    bgfx::frame();

    // check if should close
    if(glfwGetKey(mWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(mWindow, GL_TRUE);

    // handle resizing
    int w, h;
    glfwGetWindowSize(mWindow, &w, &h);
    if(w != mWidth || h != mHeight) {
        mWidth  = w;
        mHeight = h;
        reset(mReset);
    }
}

void Application::reset(uint32_t flags) {
    mReset = flags;
    bgfx::reset(mWidth, mHeight, mReset);
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xff00ffff, 1.0f, 0);
    bgfx::setViewRect(0, 0, 0, uint16_t(getWidth()), uint16_t(getHeight()));
}
