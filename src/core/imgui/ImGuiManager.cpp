#include "ImGuiManager.h"

#include "core/GraphicsHelpers.h"
#include "core/Application.h"

#include <GLFW/glfw3.h>

#if !defined(__EMSCRIPTEN__)
#   if defined(_WIN32)
#       define GLFW_EXPOSE_NATIVE_WIN32
#   endif
#   include <GLFW/glfw3native.h>
#endif

HA_SINGLETON_INSTANCE(ImGuiManager);

static bgfx_vertex_decl    ivd;
static bgfx_texture_handle imguiFontTexture;
static bgfx_uniform_handle imguiFontUniform;
static bgfx_program_handle imguiProgram;
static void                imguiInit();
static void                imguiRender(ImDrawData* drawData);
static void                imguiShutdown();
static cstr                imguiGetClipboardText(void* userData);
static void                imguiSetClipboardText(void* userData, cstr text);

ImGuiManager::ImGuiManager()
    : Singleton(this)
{
    imguiInit();
}

ImGuiManager::~ImGuiManager()
{
    imguiShutdown();
}

void ImGuiManager::update(float dt)
{
    Application& app = Application::get();
    ImGuiIO&     io = ImGui::GetIO();
    io.DeltaTime = dt;
    int w, h;
    int displayW, displayH;
    glfwGetWindowSize(app.m_window, &w, &h);
    glfwGetFramebufferSize(app.m_window, &displayW, &displayH);
    io.DisplaySize = ImVec2(float(w), float(h));
    io.IniFilename = ""; //"imgui.ini";
    io.DisplayFramebufferScale = ImVec2(float(displayW) / w, float(displayH) / h);
    double mouse_x, mouse_y;
    glfwGetCursorPos(app.m_window, &mouse_x, &mouse_y);
    io.MousePos = ImVec2(float(mouse_x), float(mouse_y));
    for (int i = 0; i < 3; i++) {
        io.MouseDown[i] = app.m_mousePressed[i] || glfwGetMouseButton(app.m_window, i) != 0;
        app.m_mousePressed[i] = false;
    }
    io.MouseWheel = app.m_mouseWheel;
    app.m_mouseWheel = 0.0f;
    glfwSetInputMode(app.m_window, GLFW_CURSOR,
        io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
    io.ClipboardUserData = app.m_window;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(app.m_window);
#endif // _WIN32

}

void ImGuiManager::onGlfwKeyEvent(int key, int action)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;

    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void ImGuiManager::onCharEvent(unsigned c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

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
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;
    io.SetClipboardTextFn = imguiSetClipboardText;
    io.GetClipboardTextFn = imguiGetClipboardText;

    ImGuiStyle& style = ImGui::GetStyle();
    style.IndentSpacing = 10.f;
}



static void imguiRender(ImDrawData* drawData) {
    for (int ii = 0, num = drawData->CmdListsCount; ii < num; ++ii) {
        bgfx_transient_vertex_buffer tvb;
        bgfx_transient_index_buffer  tib;

        const ImDrawList* drawList = drawData->CmdLists[ii];
        uint32            numVertices = (uint32)drawList->VtxBuffer.size();
        uint32            numIndices = (uint32)drawList->IdxBuffer.size();

        if (!bgfx_get_avail_transient_vertex_buffer(numVertices, &ivd) ||
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
        for (const ImDrawCmd *cmd = drawList->CmdBuffer.begin(), *cmdEnd = drawList->CmdBuffer.end();
            cmd != cmdEnd; ++cmd) {
            if (cmd->UserCallback) {
                cmd->UserCallback(drawList, cmd);
            }
            else if (0 != cmd->ElemCount) {
                uint64_t state = BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_MSAA;
                bgfx_texture_handle th = imguiFontTexture;
                if (cmd->TextureId != nullptr) {
                    union
                    {
                        ImTextureID ptr;
                        struct
                        {
                            uint16              flags;
                            bgfx_texture_handle handle;
                        } s;
                    } texture = { cmd->TextureId };
                    HA_SUPPRESS_WARNINGS
                        state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA,
                            BGFX_STATE_BLEND_INV_SRC_ALPHA);
                    HA_SUPPRESS_WARNINGS_END
                        th = texture.s.handle;
                }
                else {
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
                bgfx_submit(1, imguiProgram, 0, false);
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
