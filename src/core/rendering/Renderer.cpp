#include "Renderer.h"

#include "core/rendering/Shader.h"
#include "core/rendering/Texture.h"
#include "core/rendering/GPUProgram.h"
#include "core/rendering/GraphicsHelpers.h"
#include "core/rendering/GLSentries.h"

HA_SINGLETON_INSTANCE(Renderer);

static const char* color_vs =
"\
uniform mat4 pv; \
uniform mat4 world; \
attribute vec3 v_pos; \
attribute vec4 v_color; \
varying vec4 color; \
void main(void) \
{ \
    color = v_color; \
    gl_Position = pv * world * vec4(v_pos, 1.0); \
} \
";

static const char* color_ps =
"\
#version 100 \n\
precision mediump float; \
varying vec4 color; \
void main(void) \
{ \
    gl_FragColor = color; \
} \
";

static int Attrib_Position, Attrib_Color;

Renderer::Renderer()
    : Singleton(this)
{
    // Shaders, program and uniforms
    auto vs = std::make_shared<Shader>(ShaderType::Vertex, "main vs");
    vs->load(color_vs);
    auto fs = std::make_shared<Shader>(ShaderType::Fragment, "main frag shader");
    fs->load(color_ps);

    m_gpuProgram = std::make_shared<GPUProgram>("imgui program");

    m_gpuProgram->attachShader(vs);
    m_gpuProgram->attachShader(fs);

    Attrib_Position = m_gpuProgram->bindAttribute("v_pos");
    Attrib_Color = m_gpuProgram->bindAttribute("v_color");

    m_gpuProgram->link();

    m_projViewParam = m_gpuProgram->getParameterByName("pv");
    m_worldParam = m_gpuProgram->getParameterByName("world");

    glGenBuffers(1, &m_tmpVb);
    glGenBuffers(1, &m_tmpIb);
}

Renderer::~Renderer()
{
    glDeleteBuffers(1, &m_tmpVb);
    glDeleteBuffers(1, &m_tmpIb);
}

std::vector<renderPart>& Renderer::getRenderData(size_t layer)
{
    if (layer >= m_layers.size())
    {
        m_layers.resize(layer + 1);
    }

    return m_layers[layer];
}

void Renderer::render()
{
    m_gpuProgram->use();
    HA_GL_SENTRY(GLEnableAttrib, Attrib_Position);
    HA_GL_SENTRY(GLEnableAttrib, Attrib_Color);

    TempMesh* tmp = nullptr;

    for (auto& layer : m_layers)
    {
        m_gpuProgram->setParameter(m_projViewParam, m_projView);
        for (auto& parts : layer)
        {
            if (parts.tmpMesh.vertices) tmp = &parts.tmpMesh;
            if (!parts.geom.isValid()) continue;
            m_gpuProgram->setParameter(m_worldParam, parts.transform);

            auto& mesh = parts.geom.get();
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbh);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibh);

            GLsizei stride;
            if (mesh.normals)
            {
                stride = sizeof(float) * 10;
                glVertexAttribPointer(Attrib_Color, 4, GL_FLOAT, GL_FALSE, stride, HA_OFFSET(sizeof(float) * 6));
            }
            else
            {
                stride = sizeof(float) * 3 + sizeof(uint32);
                glVertexAttribPointer(Attrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, HA_OFFSET(sizeof(float) * 3));
            }

            glVertexAttribPointer(Attrib_Position, 3, GL_FLOAT, GL_FALSE, stride, 0);

            if (mesh.ibh)
            {
                glDrawElements(mesh.primitiveType, mesh.count, GL_UNSIGNED_SHORT, nullptr);
            }
            else
            {
                glDrawArrays(mesh.primitiveType, 0, mesh.count);
            }
        }
    }

    glClear(GL_DEPTH_BUFFER_BIT);

    if (tmp)
    {
        glBindBuffer(GL_ARRAY_BUFFER, m_tmpVb);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex::pnc) * tmp->vertices->size(), tmp->vertices->data(), GL_DYNAMIC_DRAW);

        if (tmp->indices)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_tmpIb);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * tmp->indices->size(), tmp->indices->data(), GL_DYNAMIC_DRAW);
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        GLsizei stride = sizeof(float) * 10;
        glVertexAttribPointer(Attrib_Position, 3, GL_FLOAT, GL_FALSE, stride, 0);
        glVertexAttribPointer(Attrib_Color, 4, GL_FLOAT, GL_FALSE, stride, HA_OFFSET(sizeof(float) * 6));

        if (tmp->indices)
        {
            glDrawElements(GL_TRIANGLES, GLsizei(tmp->indices->size()), GL_UNSIGNED_INT, nullptr);
        }
        else
        {
            glDrawArrays(GL_TRIANGLES, 0, GLsizei(tmp->vertices->size()));
        }
    }

    for (auto& layer : m_layers)
    {
        layer.clear();
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}