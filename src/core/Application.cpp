#include "Application.h"
#include "PluginManager.h"
#include "World.h"
#include "GraphicsHelpers.h"
#include "imgui/ImGuiManager.h"

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
// == APPLICATION INPUT ============================================================================
// =================================================================================================

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    Application* app = (Application*)glfwGetWindowUserPointer(window);
    if(action == GLFW_PRESS && button >= 0 && button < 3)
        app->m_mousePressed[button] = true;

    InputEvent ev;
    ev.button = {InputEvent::BUTTON, MouseButton(button), ButtonAction(action)};
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
    ev.scroll = {InputEvent::SCROLL, float(yoffset)};
    Application::get().addInputEvent(ev);
}

void Application::keyCallback(GLFWwindow*, int key, int, int action, int mods) {
    ImGuiManager::get().onGlfwKeyEvent(key, action);

    InputEvent ev;
    ev.key = {InputEvent::KEY, key, KeyAction(action), mods};
    Application::get().addInputEvent(ev);
}

void Application::charCallback(GLFWwindow*, unsigned int c) {
    ImGuiManager::get().onCharEvent(c);
}

void Application::cursorPosCallback(GLFWwindow*, double x, double y) {
    static double last_x = 0.0;
    static double last_y = 0.0;
    InputEvent    ev;
    ev.mouse = {InputEvent::MOUSE, float(x), float(y), float(x - last_x), float(y - last_y)};
    last_x   = x;
    last_y   = y;

    Application::get().addInputEvent(ev);
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
    reset();

    u_time = bgfx_create_uniform("u_time", BGFX_UNIFORM_TYPE_VEC4, 1);

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

        ImGuiManager imguiManager;

#ifdef EMSCRIPTEN
        emscripten_set_main_loop([]() { Application::get().update(); }, 0, 1);
#else  // EMSCRIPTEN
        while(!glfwWindowShouldClose(m_window))
            update();
#endif // EMSCRIPTEN

        // set the state to something other than EDITOR
        Application::get().setState(Application::State::PLAY);
    }

    bgfx_destroy_uniform(u_time);

    bgfx_shutdown();
    glfwTerminate();
    return tests_res;
}

void Application::processEvents() {
    // leave it here for now. Decide whether to replace it later...
    ImGuiIO& io = ImGui::GetIO();

    for(size_t i = 0; i < m_inputs.size(); ++i)
        for(auto& curr : m_inputEventListeners)
            if((m_inputs[i].type == InputEvent::KEY && !io.WantCaptureKeyboard) ||
               (m_inputs[i].type == InputEvent::MOUSE && !io.WantCaptureMouse) ||
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

    bgfx_set_uniform(u_time, &m_time, 1);

    // poll for events - also dispatches to imgui
    glfwPollEvents();
    
    ImGuiManager::get().update(m_dt);

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
    bgfx_set_view_clear(1, BGFX_CLEAR_DEPTH, 0, 1.0f, 0);
    bgfx_set_view_rect(1, 0, 0, uint16(width()), uint16(height()));
}
