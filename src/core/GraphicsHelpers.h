#pragma once

#include <bgfx/bgfx.h>

const bgfx::Memory* loadMemory(const char* filename);
bgfx::ShaderHandle loadShader(const char* shader);
bgfx::ProgramHandle loadProgram(const char* vsName, const char* fsName);

bgfx::TextureHandle loadTexture(const char* _name, uint32_t _flags = BGFX_TEXTURE_NONE,
                                uint8_t _skip = 0, bgfx::TextureInfo* _info = nullptr);

struct Mesh;

Mesh* meshLoad(const char* _filePath);
void meshUnload(Mesh* _mesh);
void meshSubmit(const Mesh* _mesh, uint8_t _id, bgfx::ProgramHandle _program, const float* _mtx,
                uint64_t _state = BGFX_STATE_MASK);
