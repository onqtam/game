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
    bgfx_vertex_buffer_handle vbh;
    bgfx_index_buffer_handle  ibh;
};

ha_mesh createCube();

struct GeomCreator
{
    template <typename... Args>
    void create(void* storage, const std::string& name, Args&&... args) {
        if(name == "cube")
            new(storage) ha_mesh(createCube(std::forward<Args>(args)...));
        else
            new(storage) ha_mesh({BGFX_INVALID_HANDLE, BGFX_INVALID_HANDLE});
    }
    void destroy(void* storage) {
        //if(bgfx::isValid(static_cast<ha_mesh*>(storage)->vbh))
        bgfx_destroy_vertex_buffer(static_cast<ha_mesh*>(storage)->vbh);
        //if(bgfx::isValid(static_cast<ha_mesh*>(storage)->ibh))
        bgfx_destroy_index_buffer(static_cast<ha_mesh*>(storage)->ibh);
    }
};

template class ResourceManager<ha_mesh, GeomCreator>;
typedef ResourceManager<ha_mesh, GeomCreator>         GeomMan;
typedef ResourceManager<ha_mesh, GeomCreator>::Handle GeomHandle;

// HACKS until bgfx can be built as a dll and used by the plugins freely
//namespace my_bgfx
//{
//inline HAPI uint32_t setTransform(const void* _mtx, uint16_t _num = 1) {
//    return bgfx::setTransform(_mtx, _num);
//}
//
//inline HAPI void destroyVertexBuffer(bgfx::VertexBufferHandle _handle) {
//    bgfx::destroyVertexBuffer(_handle);
//}
//inline HAPI void destroyIndexBuffer(bgfx::IndexBufferHandle _handle) {
//    bgfx::destroyIndexBuffer(_handle);
//}
//
//inline HAPI bgfx::VertexBufferHandle createVertexBuffer(const bgfx::Memory*     _mem,
//                                                        const bgfx_vertex_decl& _decl,
//                                                        uint16_t _flags = BGFX_BUFFER_NONE) {
//    return bgfx::createVertexBuffer(_mem, _decl, _flags);
//}
//
//inline HAPI bgfx::IndexBufferHandle createIndexBuffer(const bgfx::Memory* _mem,
//                                                      uint16_t _flags = BGFX_BUFFER_NONE) {
//    return bgfx::createIndexBuffer(_mem, _flags);
//}
//
//inline HAPI const bgfx::Memory* makeRef(const void* _data, uint32_t _size,
//                                        bgfx::ReleaseFn _releaseFn = NULL, void* _userData = NULL) {
//    return bgfx::makeRef(_data, _size, _releaseFn, _userData);
//}
//
//inline HAPI void setVertexBuffer(uint8_t _stream, bgfx::VertexBufferHandle _handle) {
//    bgfx::setVertexBuffer(_stream, _handle);
//}
//
//inline HAPI void setIndexBuffer(bgfx::IndexBufferHandle _handle) { bgfx::setIndexBuffer(_handle); }
//
//inline HAPI void setState(uint64_t _state, uint32_t _rgba = 0) { bgfx::setState(_state, _rgba); }
//
//inline HAPI uint32_t submit(uint8_t _id, bgfx_program_handle _program, int32_t _depth = 0,
//                            bool _preserveState = false) {
//    return bgfx::submit(_id, _program, _depth, _preserveState);
//}
//
//} // namespace my_bgfx
