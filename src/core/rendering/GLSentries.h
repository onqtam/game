#pragma once

//  RAII sentries for opengl

namespace gl_sentries
{
struct GLEnable
{
    GLEnable(GLenum op) : operation(op)
    {
        glEnable(operation);
    }

    ~GLEnable()
    {
        glDisable(operation);
    }

    const GLenum operation;
};

struct GLDisable
{
    GLDisable(GLenum op) : operation(op)
    {
        glDisable(operation);
    }

    ~GLDisable()
    {
        glEnable(operation);
    }

    const GLenum operation;
};

struct GLEnableAttrib
{
    GLEnableAttrib(int i) : index(i)
    {
        glEnableVertexAttribArray(index);
    }

    ~GLEnableAttrib()
    {
        glDisableVertexAttribArray(index);
    }

    const int index;
};

struct GLDepthWrite
{
    GLDepthWrite(bool write)
        : m_write(write)
    {
        glDepthMask(m_write);
    }

    ~GLDepthWrite()
    {
        glDepthMask(!m_write);
    }

    bool m_write;
};
}

#define HA_GL_SENTRY(sentry, op) ::gl_sentries::sentry BOOST_PP_CAT(sentry, __LINE__)(op)
