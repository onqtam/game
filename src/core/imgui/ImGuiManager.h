#pragma once

#include "core/rendering/GPUProgramPtr.h"
#include "core/rendering/TexturePtr.h"

struct ImDrawData;
struct ImFont;

class ImGuiManager : public Singleton<ImGuiManager>
{
public:
    ImGuiManager();
    ~ImGuiManager();

    void update(float dt);

    void onGlfwKeyEvent(int key, int action);
    void onCharEvent(unsigned c);
    
    ImFont* getMainFont() const;
    ImFont* getBigFont() const;

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
    
    ImFont* main;
    ImFont* big;

    static void imguiRenderCallback(ImDrawData* data);
};
