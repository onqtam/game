#pragma once

#include "core/ResourceManager.h"

HAPI const bgfx::Memory* loadMemory(const char* filename);
HAPI bgfx::ShaderHandle loadShader(const char* shader);
HAPI bgfx::ProgramHandle loadProgram(const char* vsName, const char* fsName);

HAPI bgfx::TextureHandle loadTexture(const char* _name, uint32 _flags = BGFX_TEXTURE_NONE,
                                     uint8_t _skip = 0, bgfx::TextureInfo* _info = nullptr);

struct Mesh;

HAPI Mesh* meshLoad(const char* _filePath);
HAPI void meshUnload(Mesh* _mesh);
HAPI void meshSubmit(const Mesh* _mesh, uint8_t _id, bgfx::ProgramHandle _program,
                     const float* _mtx, uint64_t _state = BGFX_STATE_MASK);

// =============== GRAPHICAL RESOURCE MANAGERS =================

struct ProgramHandleCreator
{
    void create(void* storage, const std::string& name) {
        new(storage)
                bgfx::ProgramHandle(loadProgram((name + "_vs").c_str(), (name + "_fs").c_str()));
    }
    void destroy(void* storage) {
        bgfx::destroyProgram(*static_cast<bgfx::ProgramHandle*>(storage));
    }
};

template class ResourceManager<bgfx::ProgramHandle, ProgramHandleCreator>;
typedef ResourceManager<bgfx::ProgramHandle, ProgramHandleCreator>         ShaderMan;
typedef ResourceManager<bgfx::ProgramHandle, ProgramHandleCreator>::Handle ShaderHandle;

struct TextureCreator
{
    void create(void* storage, const std::string& name) {
        new(storage) bgfx::TextureHandle(loadTexture(name.c_str()));
    }
    void destroy(void* storage) {
        bgfx::destroyTexture(*static_cast<bgfx::TextureHandle*>(storage));
    }
};

template class ResourceManager<bgfx::TextureHandle, TextureCreator>;
typedef ResourceManager<bgfx::TextureHandle, TextureCreator>         TextureMan;
typedef ResourceManager<bgfx::TextureHandle, TextureCreator>::Handle TextureHandle;

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
