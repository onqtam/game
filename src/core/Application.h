#pragma once

#include <bgfx/bgfx.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

#include "core/ObjectManager.h"

// bgfx utils
const bgfx::Memory* loadMemory(const char* filename);
bgfx::ShaderHandle loadShader(const char* shader);
bgfx::ProgramHandle loadProgram(const char* vsName, const char* fsName);

// application
class Application
{
    HARDLY_SCOPED_SINGLETON(Application, int main(int, char**));

    ObjectManager m_objectManager;

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow*, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow*, int key, int, int action, int mods);
    static void charCallback(GLFWwindow*, unsigned int c);

    void imguiEvents(float dt);

public:
    int run(int argc, char** argv, bgfx::RendererType::Enum type = bgfx::RendererType::Count,
            uint16_t vendorId = BGFX_PCI_ID_NONE, uint16_t deviceId = 0,
            bgfx::CallbackI* callback = NULL, bx::AllocatorI* allocator = NULL);

    void reset(uint32_t flags = 0);
    uint32_t getWidth();
    uint32_t getHeight();

    void initialize(int argc, char** argv);
    void update(float dt);
    int  shutdown() { return 0; }
    void onReset();

protected:
    GLFWwindow* mWindow;

private:
    uint32_t mReset;
    uint32_t mWidth;
    uint32_t mHeight;
    bool     mMousePressed[3] = {false, false, false};
    float    mMouseWheel      = 0.0f;
};

// vertex declarations
struct PosColorVertex
{
    float                   x;
    float                   y;
    float                   z;
    uint32_t                abgr;
    static void             init();
    static bgfx::VertexDecl ms_decl;
};
