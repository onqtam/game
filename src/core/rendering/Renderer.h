#pragma once

#include "core/messages/messages_rendering.h"
#include "core/rendering/GPUProgramPtr.h"


class Renderer : public Singleton<Renderer>
{
public:
    Renderer();
    ~Renderer();

    void setProjView(const yama::matrix& viewProj) { m_projView = viewProj; }

    std::vector<renderPart>& getRenderData(size_t layer);

    void render();

private:
    HA_SINGLETON(Renderer);

    std::vector<std::vector<renderPart>> m_layers;

    GPUProgramPtr m_gpuProgram;
    // uniforms
    int m_projViewParam = -1;
    int m_worldParam = -1;

    yama::matrix m_projView;

    GLuint m_tmpVb = 0;
    GLuint m_tmpIb = 0;
};
