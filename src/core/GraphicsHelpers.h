#pragma once

#include <bgfx/bgfx.h>

const bgfx::Memory* loadMemory(const char* filename);
bgfx::ShaderHandle loadShader(const char* shader);
bgfx::ProgramHandle loadProgram(const char* vsName, const char* fsName);
