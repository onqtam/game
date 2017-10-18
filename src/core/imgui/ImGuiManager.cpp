#include "ImGuiManager.h"

#include "core/rendering/Shader.h"
#include "core/rendering/Texture.h"
#include "core/rendering/GPUProgram.h"
#include "core/rendering/GraphicsHelpers.h"
#include "core/rendering/GLSentries.h"
#include "core/Application.h"

#include <GLFW/glfw3.h>

#if !defined(__EMSCRIPTEN__)
#   if defined(_WIN32)
#       define GLFW_EXPOSE_NATIVE_WIN32
#   endif
#   include <GLFW/glfw3native.h>
#endif

HA_SINGLETON_INSTANCE(ImGuiManager);

using namespace yama;

static cstr imguiGetClipboardText(void* userData);
static void imguiSetClipboardText(void* userData, cstr text);

static const char* vertexShaderSource = R"delim(
uniform mat4 proj;
attribute vec2 v_pos;
attribute vec2 v_texCoord;
attribute vec4 v_color;
varying vec2 texCoord;
varying vec4 color;
void main(void)
{
    texCoord = v_texCoord;
    color = v_color;
    gl_Position = proj * vec4(v_pos.xy, 0.0, 1.0);
}
)delim";

static const char* fragmentShaderSource = R"delim(
uniform sampler2D tex;
varying vec2 texCoord;
varying vec4 color;
void main(void)
{
    gl_FragColor = color * texture2D(tex, texCoord);
}
)delim";

static int Attrib_Position, Attrib_TexCoord, Attrib_Color;

ImGuiManager::ImGuiManager()
    : Singleton(this)
{
    ImGuiIO&       io = ImGui::GetIO();

    // Shaders, program and uniforms
    auto vs = std::make_shared<Shader>(ShaderType::Vertex, "imgui vertex shader");
    vs->load(vertexShaderSource);
    auto fs = std::make_shared<Shader>(ShaderType::Fragment, "imgui fragment shader");
    fs->load(fragmentShaderSource);

    m_gpuProgram = std::make_shared<GPUProgram>("imgui program");

    m_gpuProgram->attachShader(vs);
    m_gpuProgram->attachShader(fs);

    Attrib_Position = m_gpuProgram->bindAttribute("v_pos");
    Attrib_TexCoord = m_gpuProgram->bindAttribute("v_texCoord");
    Attrib_Color = m_gpuProgram->bindAttribute("v_color");

    m_gpuProgram->link();

    m_projParam = m_gpuProgram->getParameterByName("proj");
    m_textureParam = m_gpuProgram->getParameterByName("tex");

    // Fonts texture
    io.Fonts->AddFontDefault();
    
    m_fontsTexture = std::make_shared<Texture>("gui fonts texture");

    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    m_fontsTexture->loadFromData(width, height, GL_RGBA, pixels);

    io.Fonts->TexID = m_fontsTexture.get();

    io.Fonts->ClearInputData();
    io.Fonts->ClearTexData();

    // Buffers
    glGenBuffers(1, &m_vertexBuffer);
    glGenBuffers(1, &m_indexBuffer);

    // Setup render callback
    io.RenderDrawListsFn = ImGuiManager::imguiRenderCallback;

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

ImGuiManager::~ImGuiManager()
{
    glDeleteBuffers(1, &m_vertexBuffer);
    glDeleteBuffers(1, &m_indexBuffer);
    ImGui::GetIO().Fonts->TexID = nullptr;
    ImGui::Shutdown();
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

void ImGuiManager::imguiRenderCallback(ImDrawData* data)
{
    if (data->CmdListsCount == 0)
        return;

    auto& gui = ImGuiManager::get();

    HA_GL_SENTRY(GLEnable, GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    HA_GL_SENTRY(GLDisable, GL_CULL_FACE);
    HA_GL_SENTRY(GLDisable, GL_DEPTH_TEST);
    HA_GL_SENTRY(GLEnable, GL_SCISSOR_TEST);

    // ortho 2d matrix
    const float w = ImGui::GetIO().DisplaySize.x;
    const float h = ImGui::GetIO().DisplaySize.y;
    auto projection = matrix::ortho_rh(0, w, h, 0, 0, 1); // note the inverted height. ImGui uses 0,0 as top left

    gui.m_gpuProgram->use();
    gui.m_gpuProgram->setParameter(gui.m_projParam, projection);

    HA_GL_SENTRY(GLEnableAttrib, Attrib_Position);
    HA_GL_SENTRY(GLEnableAttrib, Attrib_TexCoord);
    HA_GL_SENTRY(GLEnableAttrib, Attrib_Color);

    glBindBuffer(GL_ARRAY_BUFFER, gui.m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gui.m_indexBuffer);

    glVertexAttribPointer(Attrib_Position, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), HA_OFFSET_OF(ImDrawVert, pos));
    glVertexAttribPointer(Attrib_TexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), HA_OFFSET_OF(ImDrawVert, uv));
    glVertexAttribPointer(Attrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), HA_OFFSET_OF(ImDrawVert, col));

    for (int i = 0; i < data->CmdListsCount; ++i)
    {
        const auto list = data->CmdLists[i];

        glBufferData(GL_ARRAY_BUFFER, sizeof(ImDrawVert) * list->VtxBuffer.size(), list->VtxBuffer.Data, GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ImDrawIdx) * list->IdxBuffer.size(), list->IdxBuffer.Data, GL_DYNAMIC_DRAW);

        ImDrawIdx* offsetPtr = nullptr;
        for (const auto& cmd : list->CmdBuffer)
        {
            if (cmd.UserCallback)
            {
                cmd.UserCallback(list, &cmd);
            }
            else
            {
                // draw
                const Texture* t = reinterpret_cast<Texture*>(cmd.TextureId);
                assert(t);
                gui.m_gpuProgram->setParameter(gui.m_textureParam, *t);
                glScissor(int(cmd.ClipRect.x), (int)(h - cmd.ClipRect.w), int(cmd.ClipRect.z - cmd.ClipRect.x), int(cmd.ClipRect.w - cmd.ClipRect.y));
                glDrawElements(GL_TRIANGLES, cmd.ElemCount, GL_UNSIGNED_SHORT, offsetPtr);
                gui.m_gpuProgram->resetUniforms();
            }
            offsetPtr += cmd.ElemCount;
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static cstr imguiGetClipboardText(void* userData) {
    return glfwGetClipboardString((GLFWwindow*)userData);
}

static void imguiSetClipboardText(void* userData, cstr text) {
    glfwSetClipboardString((GLFWwindow*)userData, text);
}
