#include "Application.h"
#include "PluginManager.h"
#include "World.h"
#include "rendering/GraphicsHelpers.h"
#include "rendering/Renderer.h"
#include "imgui/ImGuiManager.h"

#define RCRL_LIVE_DEMO 1

HA_SUPPRESS_WARNINGS
#include <GLFW/glfw3.h>
#include <imgui.h>

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#else // EMSCRIPTEN

#if defined(__APPLE__)
#define GLFW_EXPOSE_NATIVE_COCOA
#define GLFW_EXPOSE_NATIVE_NSGL
#elif defined(__linux__) || defined(__unix__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#elif defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#endif // platforms

#if !defined(_WIN32)
#include <unistd.h> // for chdir()
#endif              // not windows

#include <GLFW/glfw3native.h>
#endif // EMSCRIPTEN

HA_SUPPRESS_WARNINGS_END

// TODO: use a smarter allocator - the important methods here are for the mixin data
class global_mixin_allocator : public dynamix::domain_allocator
{
    char* alloc_mixin_data(size_t count, const dynamix::object*) override {
        return new char[count * mixin_data_size];
    }

    void dealloc_mixin_data(char* ptr, size_t, const dynamix::object*) override { delete[] ptr; }

    std::pair<char*, size_t> alloc_mixin(const dynamix::basic_mixin_type_info& info,
                                         const dynamix::object*) override {
        size_t size   = mem_size_for_mixin(info.size, info.alignment);
        char*  buffer = new char[size];

        size_t offset = mixin_offset(buffer, info.alignment);

        return std::make_pair(buffer, offset);
    }

    void dealloc_mixin(char* ptr, size_t, const dynamix::basic_mixin_type_info&,
                       const dynamix::object*) override {
        delete[] ptr;
    }
};

static ppk::assert::implementation::AssertAction::AssertAction assertHandler(
        cstr file, int line, cstr function, cstr expression, int level, cstr message) {
    using namespace ppk::assert::implementation;

    char buf[2048];

    size_t num_written =
            snprintf(buf, HA_COUNT_OF(buf),
                     "Assert failed!\nFile: %s\nLine: %d\nFunction: %s\nExpression: %s", file, line,
                     function, expression);

    if(message)
        snprintf(buf + num_written, HA_COUNT_OF(buf) - num_written, "Message: %s\n", message);

    fprintf(stderr, "%s", buf);

    if(level < AssertLevel::Debug) {
        return static_cast<AssertAction::AssertAction>(0);
    } else if(AssertLevel::Debug <= level && level < AssertLevel::Error) {
#ifdef _WIN32
        // this might cause issues if there are multiple windows or threads
        int res = MessageBox(GetActiveWindow(), buf, "Assert failed! Break in the debugger?",
                             MB_YESNO | MB_ICONEXCLAMATION);

        if(res == 7)
            return static_cast<AssertAction::AssertAction>(0);
#endif
        return AssertAction::Break;
    } else if(AssertLevel::Error <= level && level < AssertLevel::Fatal) {
        return AssertAction::Throw;
    }

    return AssertAction::Abort;
}

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

bool g_console_visible = false;

void Application::keyCallback(GLFWwindow*, int key, int, int action, int mods) {
    // calling the callback from the imgui/glfw integration only if not a dash because when writing an underscore (with shift down)
    // ImGuiColorTextEdit does a paste - see this for more info: https://github.com/BalazsJako/ImGuiColorTextEdit/issues/18
    if(key != '-' || g_console_visible == false) {
        ImGuiManager::get().onGlfwKeyEvent(key, action);

        InputEvent ev;
        ev.key = {InputEvent::KEY, key, KeyAction(action), mods};
        Application::get().addInputEvent(ev);
    }

    ImGuiIO& io = ImGui::GetIO();

    if(g_console_visible) {
        // add the '\n' char when 'enter' is pressed - for ImGuiColorTextEdit
        if(io.WantCaptureKeyboard && key == GLFW_KEY_ENTER && !io.KeyCtrl &&
           (action == GLFW_PRESS || action == GLFW_REPEAT))
            io.AddInputCharacter((unsigned short)'\n');
    }

    // console toggle
    if(!io.WantCaptureKeyboard && !io.WantTextInput && key == GLFW_KEY_GRAVE_ACCENT &&
       (action == GLFW_PRESS || action == GLFW_REPEAT))
        g_console_visible = !g_console_visible;
}

void Application::charCallback(GLFWwindow*, unsigned int c) { ImGuiManager::get().onCharEvent(c); }

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

    ppk::assert::implementation::setAssertHandler(assertHandler);

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
    m_width                    = mode->width * 3 / 4;
    m_height                   = mode->height * 3 / 4;
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
    // Simulating fullscreen - by placing the window in 0,0
    int monitor_pos_x, monitor_pos_y;
    glfwGetMonitorPos(monitor, &monitor_pos_x, &monitor_pos_y);
    glfwSetWindowPos(m_window, monitor_pos_x, monitor_pos_y);
    //glfwSetWindowMonitor(m_window, monitor, 0, 0, width(), height(), mode->refreshRate);
#endif // EMSCRIPTEN

    // Setup input callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwMakeContextCurrent(m_window);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetCharCallback(m_window, charCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    glfwSetCursorPos(m_window, width() / 2, height() / 2);

    // Setup rendering
#if defined(_WIN32)
    auto glewInitResult = glewInit();
    if(glewInitResult != GLEW_OK) {
        fprintf(stderr, "Couldn't initialize glew. Reason: %s\n",
                glewGetErrorString(glewInitResult));
        glfwTerminate();
        return -1;
    }
#endif
    glEnable(GL_DEPTH_TEST); // z buffer
    glEnable(GL_CULL_FACE);  // cull back (CW) faces
    glClearColor(0.75f, 0.05f, 0.65f, 1);

    // Initialize the application
    reset();

    // introduce this scope in order to control the lifetimes of managers
    {
        // resource managers should be created first and destroyed last - all
        // objects should be destroyed so the refcounts to the resources are 0
        ShaderMan shaderMan;
        GeomMan   geomMan;

        ObjectManager objectMan;

        World world;

        ImGuiManager imguiManager;
        Renderer     renderer;

#ifdef EMSCRIPTEN
        emscripten_set_main_loop([]() { Application::get().update(); }, 0, 1);
#else  // EMSCRIPTEN
        while(!glfwWindowShouldClose(m_window))
            update();
#endif // EMSCRIPTEN

        // set the state to something other than EDITOR
        Application::get().setState(Application::State::PLAY);
    }

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

    // poll for events - also dispatches to imgui
    glfwPollEvents();

    ImGuiManager::get().update(m_dt);

    // imgui
    ImGui::NewFrame();

    // send input events to the rest of the app
    processEvents();

    int w, h;
    glfwGetWindowSize(m_window, &w, &h);

    // main imgui font
    ImGui::PushFont(ImGuiManager::get().getMainFont());

    // update game stuff
    World::get().update();

    // pop main font
    ImGui::PopFont();

#if defined(HA_WITH_PLUGINS) && defined(_MSC_VER)
    void do_rcrl(int);
    do_rcrl(w);
#endif // HA_WITH_PLUGINS && MSVC

    // render
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, m_width, m_height);
    Renderer::get().render();
    ImGui::Render();
    glfwSwapBuffers(m_window);

    // handle resizing
    if(uint32(w) != m_width || uint32(h) != m_height) {
        m_width  = w;
        m_height = h;
        reset(m_reset);
    }
}

void Application::reset(uint32 flags) { m_reset = flags; }

#if defined(HA_WITH_PLUGINS) && defined(_MSC_VER)

#include <ImGuiColorTextEdit/TextEditor.h>
#include "rcrl/rcrl.h"

void do_rcrl(int window_w) {
    // push big font for rcrl
    ImGui::PushFont(ImGuiManager::get().getBigFont());

    ImGuiIO& io = ImGui::GetIO();

    static TextEditor history;
    static TextEditor compiler_output;
    static TextEditor program_output(compiler_output);
    static TextEditor editor;
    static int        last_compiler_exitcode = 0;
    static bool       used_default_mode      = false;
    static rcrl::Mode default_mode           = rcrl::ONCE;

    static bool inited = false;
    if(!inited) {
        inited = true;

        history.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
        history.SetReadOnly(true);
        history.SetText("#include \"precompiled.h\"\n");

        compiler_output.SetLanguageDefinition(TextEditor::LanguageDefinition());
        compiler_output.SetReadOnly(true);
        auto custom_palette = TextEditor::GetDarkPalette();
        custom_palette[(int)TextEditor::PaletteIndex::MultiLineComment] =
                0xcccccccc; // text is treated as such by default
        compiler_output.SetPalette(custom_palette);

        editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());

#if !RCRL_LIVE_DEMO
        // set some initial code
        editor.SetText(R"raw(// vars
auto& objects = ObjectManager::get().getObjects();
// once
for(auto& obj : objects)
    obj.second.move_local({20, 0, 0});
)raw");
#endif // RCRL_LIVE_DEMO
    }

    // console should be always fixed
    ImGui::SetNextWindowSize({(float)window_w, -1.f}, ImGuiCond_Always);
    ImGui::SetNextWindowPos({0.f, 0.f}, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(1);

    // sets breakpoints on the program_output instance of the text editor widget - used to highlight new output
    auto do_breakpoints_on_output = [&](int old_line_count, const std::string& new_output) {
        TextEditor::Breakpoints bps;
        if(old_line_count == program_output.GetTotalLines() && new_output.size())
            bps.insert(old_line_count);
        for(auto curr_line = old_line_count; curr_line < program_output.GetTotalLines();
            ++curr_line)
            bps.insert(curr_line);
        program_output.SetBreakpoints(bps);
    };

    if(g_console_visible && ImGui::Begin("console", nullptr,
                                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                                 ImGuiWindowFlags_NoMove)) {
        const auto  text_field_height = ImGui::GetTextLineHeight() * 13;
        const float left_right_ratio  = 0.5f;
        // top left part
        ImGui::BeginChild("history code", ImVec2(window_w * left_right_ratio, text_field_height));
        auto hcpos = history.GetCursorPosition();
        ImGui::Text("Executed code: %3d/%-3d %3d lines", hcpos.mLine + 1, hcpos.mColumn + 1,
                    editor.GetTotalLines());
        history.Render("History");
        ImGui::EndChild();
        ImGui::SameLine();
        // top right part
        ImGui::BeginChild("compiler output", ImVec2(0, text_field_height));
        auto new_output = rcrl::get_new_compiler_output();
        if(new_output.size()) {
            auto total_output = compiler_output.GetText() + new_output;

            // scan for errors through the lines and highlight them with markers
            size_t                   curr_pos                = 0;
            auto                     line                    = 1;
            auto                     first_error_marker_line = 0;
            TextEditor::ErrorMarkers error_markers;
            do {
                auto new_curr_pos_1 =
                        total_output.find("error", curr_pos + 1); // add 1 to skip new lines
                auto new_curr_pos_2 =
                        total_output.find("\n", curr_pos + 1); // add 1 to skip new lines
                if(new_curr_pos_1 < new_curr_pos_2) {
                    error_markers.insert(std::make_pair(line, ""));
                    if(!first_error_marker_line)
                        first_error_marker_line = line;
                }
                if(new_curr_pos_2 < new_curr_pos_1) {
                    line++;
                }
                curr_pos = std::min(new_curr_pos_1, new_curr_pos_2);
            } while(size_t(curr_pos) != std::string::npos);
            compiler_output.SetErrorMarkers(error_markers);

            // update compiler output
            compiler_output.SetText(move(total_output));
            if(first_error_marker_line)
                compiler_output.SetCursorPosition({first_error_marker_line, 1});
            else
                compiler_output.SetCursorPosition({compiler_output.GetTotalLines(), 1});
        }
        if(last_compiler_exitcode)
            ImGui::TextColored({1, 0, 0, 1}, "Compiler output - ERROR!");
        else
            ImGui::Text("Compiler output:        ");
        ImGui::SameLine();
        auto cocpos = compiler_output.GetCursorPosition();
        ImGui::Text("%3d/%-3d %3d lines", cocpos.mLine + 1, cocpos.mColumn + 1,
                    compiler_output.GetTotalLines());
        compiler_output.Render("Compiler output");
        ImGui::EndChild();

        // bottom left part
        ImGui::BeginChild("source code", ImVec2(window_w * left_right_ratio, text_field_height));
        auto ecpos = editor.GetCursorPosition();
        ImGui::Text("RCRL Console: %3d/%-3d %3d lines | %s", ecpos.mLine + 1, ecpos.mColumn + 1,
                    editor.GetTotalLines(), editor.CanUndo() ? "*" : " ");
        editor.Render("Code");
        ImGui::EndChild();
        ImGui::SameLine();
        // bottom right part
        ImGui::BeginChild("program output", ImVec2(0, text_field_height));
        auto ocpos = program_output.GetCursorPosition();
        ImGui::Text("Program output: %3d/%-3d %3d lines", ocpos.mLine + 1, ocpos.mColumn + 1,
                    program_output.GetTotalLines());
        program_output.Render("Output");
        ImGui::EndChild();

        // bottom buttons
        ImGui::Text("Default mode:");
        ImGui::SameLine();
        ImGui::RadioButton("global", (int*)&default_mode, rcrl::GLOBAL);
        ImGui::SameLine();
        ImGui::RadioButton("vars", (int*)&default_mode, rcrl::VARS);
        ImGui::SameLine();
        ImGui::RadioButton("once", (int*)&default_mode, rcrl::ONCE);
        ImGui::SameLine();
        auto compile = ImGui::Button("Compile and run");
        ImGui::SameLine();
        if(ImGui::Button("Cleanup Plugins") && !rcrl::is_compiling()) {
            compiler_output.SetText("");
            auto output_from_cleanup = rcrl::cleanup_plugins(true);
            auto old_line_count      = program_output.GetTotalLines();
            program_output.SetText(program_output.GetText() + output_from_cleanup);
            program_output.SetCursorPosition({program_output.GetTotalLines(), 0});

            last_compiler_exitcode = 0;
            history.SetText("#include \"precompiled_for_plugin.h\"\n");

            // highlight the new stdout lines
            do_breakpoints_on_output(old_line_count, output_from_cleanup);
        }
        ImGui::SameLine();
        if(ImGui::Button("Clear Output"))
            program_output.SetText("");
        ImGui::SameLine();
        ImGui::Dummy({20, 0});
        ImGui::SameLine();
#if !RCRL_LIVE_DEMO
        ImGui::Text("Use Ctrl+Enter to submit code");
#endif // RCRL_LIVE_DEMO

        // if the user has submitted code for compilation
#if RCRL_LIVE_DEMO
        extern std::list<const char*> fragments;
        static bool                   fragment_popped = false;
        if(!rcrl::is_compiling() && !fragment_popped && fragments.size() &&
           io.KeysDown[GLFW_KEY_ENTER] && io.KeyCtrl) {
            editor.SetText(fragments.front());
            fragments.pop_front();
            fragment_popped = true;
        }
#else  // RCRL_LIVE_DEMO
        compile |= (io.KeysDown[GLFW_KEY_ENTER] && io.KeyCtrl);
#endif // RCRL_LIVE_DEMO
        if(compile && !rcrl::is_compiling() && editor.GetText().size() > 1) {
            // clear compiler output
            compiler_output.SetText("");

            auto getCodePrefix = [&]() {
#ifdef __APPLE__ // related to this: https://github.com/onqtam/rcrl/issues/4
                static bool first_time_called = true;
                if(first_time_called) {
                    first_time_called = false;
                    return string("//global\n#include \"precompiled_for_plugin.h\"\n") +
                           (default_mode == rcrl::GLOBAL ?
                                    "// global\n" :
                                    (default_mode == rcrl::VARS ? "// vars\n" : "// once\n"));
                }
#endif
                return std::string("");
            };

            // submit to the RCRL engine
            if(rcrl::submit_code(getCodePrefix() + editor.GetText(), default_mode,
                                 &used_default_mode)) {
                // make the editor code untouchable while compiling
                editor.SetReadOnly(true);
            } else {
                last_compiler_exitcode = 1;
            }
#if RCRL_LIVE_DEMO
            fragment_popped = false;
#endif // RCRL_LIVE_DEMO
        }
        ImGui::End();
    }

    // if there is a spawned compiler process and it has just finished
    if(rcrl::try_get_exit_status_from_compile(last_compiler_exitcode)) {
        // we can edit the code again
        editor.SetReadOnly(false);

        if(last_compiler_exitcode) {
            // errors occurred - set cursor to the last line of the erroneous code
            editor.SetCursorPosition({editor.GetTotalLines(), 0});
        } else {
            // append to the history and focus last line
            history.SetCursorPosition({history.GetTotalLines(), 1});
            auto history_text = history.GetText();
            // add a new line (if one is missing) to the code that will go to the history for readability
            if(history_text.size() && history_text.back() != '\n')
                history_text += '\n';
            // if the default mode was used - add an extra comment before the code to the history for clarity
            if(used_default_mode)
                history_text += default_mode == rcrl::GLOBAL ?
                                        "// global\n" :
                                        (default_mode == rcrl::VARS ? "// vars\n" : "// once\n");
            history.SetText(history_text + editor.GetText());

            // load the new plugin
            auto output_from_loading = rcrl::copy_and_load_new_plugin(true);
            auto old_line_count      = program_output.GetTotalLines();
            program_output.SetText(program_output.GetText() + output_from_loading);
            // TODO: used for auto-scroll but the cursor in the editor is removed (unfocused) sometimes from this...
            //program_output.SetCursorPosition({program_output.GetTotalLines(), 0});

            // highlight the new stdout lines
            do_breakpoints_on_output(old_line_count, output_from_loading);

            // clear the editor
            editor.SetText("\r"); // an empty string "" breaks it for some reason...
            editor.SetCursorPosition({0, 0});
        }
    }
    // pop the big badboy font
    ImGui::PopFont();
}

#if RCRL_LIVE_DEMO

std::list<const char*> fragments = {
        R"raw(// vars
auto& objects = ObjectManager::get().getObjects();
)raw",
        R"raw(for(auto& obj : objects)
    obj.second.move_local({20, 0, 0});
)raw",
        R"raw(//once
auto& objects = ObjectManager::get().getObjects();

JsonData state;
state.startObject();

state.addKey("objects");
state.startArray();

for(auto& p : objects) {
    auto& curr = p.second;

    state.startObject();
    state.addKey("id");
    auto id_str = std::to_string(oid::internal_type(curr.id()));
    state.append(id_str.c_str(), id_str.size());
    state.addComma();
    state.addKey("state");
    serialize(curr, state);
    state.addComma();
    state.addKey("mixins");
    state.startObject();
    if(curr.implements(common::serialize_mixins_msg))
        common::serialize_mixins(curr, nullptr, state);
    state.endObject();
    state.endObject();

    state.addComma();
}

state.endArray();
state.endObject();

state.prettify();

auto f = fopen("level.json", "wb");
fwrite(state.data().data(), 1, state.size(), f);
fclose(f);
)raw"};

#endif // RCRL_LIVE_DEMO

#endif // HA_WITH_PLUGINS && MSVC
