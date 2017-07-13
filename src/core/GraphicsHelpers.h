#pragma once

#include "core/ResourceManager.h"

HAPI const bgfx_memory* loadMemory(const char* filename);
HAPI bgfx_shader_handle loadShader(const char* shader);
HAPI bgfx_program_handle loadProgram(const char* vsName, const char* fsName);

HAPI bgfx_texture_handle loadTexture(const char* _name, uint32 _flags = BGFX_TEXTURE_NONE,
                                     uint8_t _skip = 0, bgfx_texture_info* _info = nullptr);

struct Mesh;

HAPI Mesh* meshLoad(const char* _filePath);
HAPI void meshUnload(Mesh* _mesh);
HAPI void meshSubmit(const Mesh* _mesh, uint8_t _id, bgfx_program_handle _program,
                     const float* _mtx, uint64_t _state = BGFX_STATE_MASK);

// =============== GRAPHICAL RESOURCE MANAGERS =================

struct ProgramHandleCreator
{
    void create(void* storage, const std::string& name) {
        new(storage)
                bgfx_program_handle(loadProgram((name + "_vs").c_str(), (name + "_fs").c_str()));
    }
    void destroy(void* storage) {
        bgfx_program_handle& hndl = *static_cast<bgfx_program_handle*>(storage);
        bgfx_destroy_program(hndl);
    }
};

template class ResourceManager<bgfx_program_handle, ProgramHandleCreator>;
typedef ResourceManager<bgfx_program_handle, ProgramHandleCreator>         ShaderMan;
typedef ResourceManager<bgfx_program_handle, ProgramHandleCreator>::Handle ShaderHandle;

struct TextureCreator
{
    void create(void* storage, const std::string& name) {
        new(storage) bgfx_texture_handle(loadTexture(name.c_str()));
    }
    void destroy(void* storage) {
        bgfx_destroy_texture(*static_cast<bgfx_texture_handle*>(storage));
    }
};

template class ResourceManager<bgfx_texture_handle, TextureCreator>;
typedef ResourceManager<bgfx_texture_handle, TextureCreator>         TextureMan;
typedef ResourceManager<bgfx_texture_handle, TextureCreator>::Handle TextureHandle;

struct MeshCreator
{
    void create(void* storage, const std::string& name) {
        new(storage) Mesh*(meshLoad(name.c_str()));
    }
    void destroy(void* storage) { meshUnload(*static_cast<Mesh**>(storage)); }
};

template class ResourceManager<Mesh*, MeshCreator>;
typedef ResourceManager<Mesh*, MeshCreator>         MeshMan;
typedef ResourceManager<Mesh*, MeshCreator>::Handle MeshHandle;

struct ha_mesh
{
    bgfx_vertex_buffer_handle vbh   = {BGFX_INVALID_HANDLE};
    bgfx_index_buffer_handle  ibh   = {BGFX_INVALID_HANDLE};
    uint64                    state = BGFX_STATE_NONE;
};

HAPI ha_mesh createCube();
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
        if(mesh.vbh.idx != BGFX_INVALID_HANDLE)
            bgfx_destroy_vertex_buffer(mesh.vbh);
        if(mesh.ibh.idx != BGFX_INVALID_HANDLE)
            bgfx_destroy_index_buffer(mesh.ibh);
    }
};

template class ResourceManager<ha_mesh, GeomCreator>;
typedef ResourceManager<ha_mesh, GeomCreator>         GeomMan;
typedef ResourceManager<ha_mesh, GeomCreator>::Handle GeomHandle;
