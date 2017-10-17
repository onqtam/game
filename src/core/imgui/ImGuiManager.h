#pragma once

#include "core/rendering/GPUProgramPtr.h"
#include "core/rendering/TexturePtr.h"

struct ImDrawData;

class ImGuiManager : public Singleton<ImGuiManager>
{
public:
    ImGuiManager();
    ~ImGuiManager();

    void update(float dt);

    void onGlfwKeyEvent(int key, int action);
    void onCharEvent(unsigned c);
    
private:
    HA_SINGLETON(ImGuiManager);

    ///////////////////////////////////////
    // rendering
    GPUProgramPtr m_gpuProgram;

    GLuint m_vertexBuffer = 0;
    GLuint m_indexBuffer = 0;

    // uniforms
    int m_textureParam = -1;
    int m_projParam = -1;

    TexturePtr m_fontsTexture;

    static void imguiRenderCallback(ImDrawData* data);
};
