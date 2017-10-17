// class for a GPU program (effect)
#include "GPUProgram.h"
#include "Shader.h"
#include "Texture.h"

#include <iostream>

using namespace std;
using namespace yama;

GPUProgram::GPUProgram()
    : GPUProgram("<unnamed gpu program>")
{

}

GPUProgram::GPUProgram(const std::string& name)
    : m_name(name)
    , m_boundAttribPtr(0)
    , m_boundTextures(0)
{
    m_glHandle = glCreateProgram();

    assert(m_glHandle);

    m_shaders.reserve(ShaderType::Count);
}

GPUProgram::~GPUProgram()
{
    if (m_glHandle)
        glDeleteProgram(m_glHandle);
}

bool GPUProgram::link()
{
    glLinkProgram(m_glHandle);

    GLint isLinked;
    glGetProgramiv(m_glHandle, GL_LINK_STATUS, &isLinked);

    if (!isLinked)
    {
        GLint infoLen = 0;
        glGetShaderiv(m_glHandle, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1)
        {
            std::unique_ptr<char[]> infoLog(new char[infoLen]);
            glGetShaderInfoLog(m_glHandle, infoLen, nullptr, infoLog.get());

            cerr << "errors linking " << m_name << ":" << endl;
            cerr << infoLog.get() << endl;
        }
        else
        {
            cerr << "unkown errors linking " << m_name;
        }

        return false;
    }

    return true;
}

void GPUProgram::attachShader(ConstShaderPtr shader)
{
    m_shaders.push_back(shader);

    glAttachShader(m_glHandle, shader->m_glHandle);
}

void GPUProgram::use()
{
    glUseProgram(m_glHandle);
    resetUniforms();
}

void GPUProgram::resetUniforms()
{
    m_boundTextures = 0;
}

uint32_t GPUProgram::bindAttribute(const char* name)
{
    glBindAttribLocation(m_glHandle, m_boundAttribPtr, name);
    return m_boundAttribPtr++;
}

void GPUProgram::bindAttributeAt(const char* name, uint32_t location)
{
    glBindAttribLocation(m_glHandle, location, name);
}

GLint GPUProgram::getAttributeLocation(const char* name) const
{
    return glGetAttribLocation(m_glHandle, name);
}

GLint GPUProgram::getParameterByName(const char* name) const
{
    return glGetUniformLocation(m_glHandle, name);
}

void GPUProgram::setParameter(GLint handle, float value)
{
    glUniform1f(handle, value);
}

void GPUProgram::setParameter(GLint param, const matrix& matrix)
{
    glUniformMatrix4fv(param, 1, false, matrix.as_ptr());
}

void GPUProgram::setParameter(GLint param, const vector2& vec)
{
    glUniform2fv(param, 1, vec.as_ptr());
}

void GPUProgram::setParameter(GLint param, const vector3& vec)
{
    glUniform3fv(param, 1, vec.as_ptr());
}

void GPUProgram::setParameter(GLint param, const vector4& vec)
{
    glUniform4fv(param, 1, vec.as_ptr());
}

void GPUProgram::setParameter(GLint param, const Texture& tex)
{
    glActiveTexture(GL_TEXTURE0 + m_boundTextures);
    glBindTexture(tex.type(), tex.glHandle());
    glUniform1i(param, m_boundTextures);

    ++m_boundTextures;
}
