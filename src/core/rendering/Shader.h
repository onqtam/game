#pragma once

#include "ShaderType.h"

// class for a shader

class Shader
{
public:
    Shader(ShaderType::Type t);
    Shader(ShaderType::Type t, const std::string& name);
    ~Shader();

    // buffer is simply the shader text
    bool load(const char* buffer);

    // load through a vector of char
    bool load(const std::vector<char>& buffer);

    ShaderType::Type type() const { return m_type; }

private:
    bool compile();

    friend class GPUProgram;

    ShaderType::Type m_type;
    std::string m_name;

    GLuint m_glHandle;
    GLenum m_glType;
};
