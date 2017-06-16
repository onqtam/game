#include "GraphicsHelpers.h"

#include <fstream>

const bgfx::Memory* loadMemory(const char* filename) {
    std::ifstream   file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    const bgfx::Memory* mem = bgfx::alloc(uint32_t(size + 1));
    if(file.read((char*)mem->data, size)) {
        mem->data[mem->size - 1] = '\0';
        return mem;
    }
    return nullptr;
}

bgfx::ShaderHandle loadShader(const char* shader) { return bgfx::createShader(loadMemory(shader)); }

bgfx::ProgramHandle loadProgram(const char* vsName, const char* fsName) {
    bgfx::ShaderHandle vs = loadShader(vsName);
    bgfx::ShaderHandle fs = loadShader(fsName);
    return bgfx::createProgram(vs, fs, true);
}
