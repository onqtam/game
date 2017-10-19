#include "Texture.h"

using namespace std;

Texture::Texture()
    : Texture("<unnamed texture>")
{
}

Texture::Texture(const std::string& name)
        : m_width(0)
        , m_height(0)
        , m_name(name) {
    glGenTextures(1, &m_glHandle);
}

Texture::~Texture() { glDeleteTextures(1, &m_glHandle); }

bool Texture::loadFromFile(const char* filename)
{
    hassert(!filename);
    return false;
}

void Texture::loadFromData(GLsizei width, GLsizei height, GLenum format, const void* data)
{
    glBindTexture(GL_TEXTURE_2D, m_glHandle);
    //glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    m_format = format;
    m_width = width;
    m_height = height;
}

void Texture::setParameter(GLenum param, GLint value)
{
    glTexParameteri(GL_TEXTURE_2D, param, value);
}
