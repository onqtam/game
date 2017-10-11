#pragma once

#include "ShaderPtr.h"

// class for a GPU program (effect)

class Texture;

class GPUProgram
{
public:
    GPUProgram();
    GPUProgram(const std::string& name); // name is used for debugging purposes
    ~GPUProgram();

    void attachShader(ConstShaderPtr shader);

    bool link();
    void use();

    void resetUniforms(); // call before adding a new set uniforms to the program

    GLint getParameterByName(const char* name) const;

    // bind attribute at a generated location
    // returns the index where the attrbute was bound
    // must link program after calling this in order for the changes to take place
    uint32_t bindAttribute(const char* name);

    // bind attribute at a user-specified location
    // must link program after calling this in order for the changes to take place
    void bindAttributeAt(const char* name, uint32_t location);

    // get attribute location
    // return -1 if no such attribute exists
    GLint getAttributeLocation(const char* name) const;

    uint32_t numBoundAttribs() const { return m_boundAttribPtr; }

    void setParameter(GLint handle, float value);
    void setParameter(GLint handle, const yama::vector2& vec);
    void setParameter(GLint handle, const yama::vector3& vec);
    void setParameter(GLint handle, const yama::vector4& vec);
    void setParameter(GLint handle, const yama::matrix& matrix);

    void setParameter(GLint handle, const Texture& tex);

private:
    const std::string m_name;

    GLuint m_glHandle;

    std::vector<ConstShaderPtr> m_shaders;
    uint32_t m_boundAttribPtr;

    uint32_t m_boundTextures;
};
