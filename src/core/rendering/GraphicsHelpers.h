#pragma once

#include "utils/aabb/aabb.h"
#include "Vertex.h"

// =============== GRAPHICAL RESOURCE MANAGERS =================

struct bs_handle {};

struct ProgramHandleCreator
{
    void create(void* storage, const std::string& name) {
        ((void)storage);
        ((void)name);
    }
    void destroy(void* storage) {
        ((void)storage);
    }
};

#define HA_RESOURCE_MANAGER ShaderMan
#define HA_RESOURCE_CREATOR ProgramHandleCreator
#define HA_RESOURCE_TYPE bs_handle
#include "core/ResourceManager.inl"
typedef ShaderMan::Handle ShaderHandle;

struct TextureCreator
{
    void create(void* storage, const std::string& name) {
        ((void)storage);
        ((void)name);
    }
    void destroy(void* storage) {
        ((void)storage);
    }
};

#define HA_RESOURCE_MANAGER TextureMan
#define HA_RESOURCE_CREATOR TextureCreator
#define HA_RESOURCE_TYPE bs_handle
#include "core/ResourceManager.inl"
typedef TextureMan::Handle TextureHandle;

struct ha_mesh
{
    ha_mesh() = default;
    ha_mesh(GLuint v, GLuint i, bool n, const AABB& bb)
        : vbh(v)
        , ibh(i)
        , normals(n)
        , bbox(bb)
    {}
    GLuint  vbh   = 0;
    GLuint  ibh   = 0;
    GLenum  primitiveType = GL_TRIANGLES;
    GLsizei count = 0;
    bool    normals = false; // has normals
    AABB    bbox;
};

HAPI ha_mesh createBox(float size_x, float size_y, float size_z, uint32 color);
HAPI ha_mesh createSolidBox(float size_x, float size_y, float size_z, uint32 color);
HAPI ha_mesh createGrid(int lines_x, int lines_z, float size_x, float size_z, uint32 color);

struct GeomCreator
{
    template <typename F, typename... Args>
    ha_mesh create2(const std::string&, F f, Args&&... args) {
        return f(std::forward<Args>(args)...);
    }

    template <typename... Args>
    void create(void* storage, const std::string& name, Args&&... args) {
        new(storage) ha_mesh(create2(name, std::forward<Args>(args)...));
    }
    void destroy(void* storage) {
        auto& mesh = *static_cast<ha_mesh*>(storage);
        if(mesh.vbh)
            glDeleteBuffers(1, &mesh.vbh);
        if(mesh.ibh)
            glDeleteBuffers(1, &mesh.ibh);
    }
};

#define HA_RESOURCE_MANAGER GeomMan
#define HA_RESOURCE_CREATOR GeomCreator
#define HA_RESOURCE_TYPE ha_mesh
#include "core/ResourceManager.inl"
typedef GeomMan::Handle GeomHandle;

namespace colors
{
const uint32 green       = 0xff00dd00;
const uint32 light_green = 0xffffffff;
} // namespace colors

struct TempMesh
{
    const std::vector<vertex::pnc>* vertices = nullptr;
    const std::vector<uint32>* indices = nullptr;
};
