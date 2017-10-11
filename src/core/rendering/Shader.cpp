#include "Shader.h"

#include <iostream>

using namespace std;

Shader::Shader(ShaderType::Type t)
    : Shader(t, "<unnamed shader>")
{
}

Shader::Shader(ShaderType::Type t, const std::string& name)
    : m_type(t)
    , m_name(name)
{
    switch (t)
    {
    case ShaderType::Vertex:
        m_glType = GL_VERTEX_SHADER;
        break;
    //case ShaderType::Geometry:
    //    m_glType = GL_GEOMETRY_SHADER;
    //    break;
    case ShaderType::Fragment:
        m_glType = GL_FRAGMENT_SHADER;
        break;
    default:
        // unknown shader type
        assert(false);
    }

    m_glHandle = glCreateShader(m_glType);

    assert(m_glHandle);
}

Shader::~Shader()
{
    if (m_glHandle)
        glDeleteShader(m_glHandle);
}

bool Shader::load(const char* buffer)
{
    glShaderSource(m_glHandle, 1, &buffer, nullptr);
    return compile();
}

bool Shader::load(const vector<char>& buffer)
{
    const char* buf = &buffer.front();
    GLint length = GLint(buffer.size());
    glShaderSource(m_glHandle, 1, &buf, &length);
    return compile();
}

bool Shader::compile()
{
    glCompileShader(m_glHandle);

    GLint isCompiled;
    glGetShaderiv(m_glHandle, GL_COMPILE_STATUS, &isCompiled);

    GLint infoLen = 0;
    glGetShaderiv(m_glHandle, GL_INFO_LOG_LENGTH, &infoLen);

    if (infoLen > 1)
    {
        std::unique_ptr<char[]> infoLog(new char[infoLen]);
        glGetShaderInfoLog(m_glHandle, infoLen, nullptr, infoLog.get());

        if (!isCompiled)
        {
            cerr << "errors compiling " << m_name << ":" << endl;
            cerr << infoLog.get() << endl;
            return false;
        }
        else
        {
            cout << "warnings compiling " << m_name << ":" << endl;
            cout << infoLog.get() << endl;
        }
    }

    return true;
}
