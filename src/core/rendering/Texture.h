#pragma once

// class for a Texture

class Texture
{
public:
    Texture();
    Texture(const std::string& name);
    ~Texture();

    bool loadFromFile(const char* filename);
    void loadFromData(GLsizei width, GLsizei height, GLenum format, const void* data);

    void setParameter(GLenum, GLint);

    GLenum type() const { return GL_TEXTURE_2D; }
    GLuint glHandle() const { return m_glHandle; }

    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    const std::string& name() const { return m_name; }

private:
    GLuint m_glHandle;
    GLuint m_format; // opengl format of the texture

    uint32_t m_width;
    uint32_t m_height;

    std::string m_name;
};
