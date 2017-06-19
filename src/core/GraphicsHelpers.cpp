#include "GraphicsHelpers.h"

#include <fstream>
#include <bx/bx.h>
#include <bx/string.h>

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

bgfx::ShaderHandle loadShader(const char* shader) {
    char filePath[512];

    const char* shaderPath = "???";

    switch(bgfx::getRendererType()) {
        case bgfx::RendererType::Noop:
        case bgfx::RendererType::Direct3D9: shaderPath = "shaders/dx9/"; break;
        case bgfx::RendererType::Direct3D11:
        case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/"; break;
        case bgfx::RendererType::Gnm: shaderPath        = "shaders/pssl/"; break;
        case bgfx::RendererType::Metal: shaderPath      = "shaders/metal/"; break;
        case bgfx::RendererType::OpenGL: shaderPath     = "shaders/glsl/"; break;
        case bgfx::RendererType::OpenGLES: shaderPath   = "shaders/gles/"; break;
        case bgfx::RendererType::Vulkan: shaderPath     = "shaders/spirv/"; break;
        case bgfx::RendererType::Count: BX_CHECK(false, "You should not be here!"); break;
    }

    bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
    bx::strCat(filePath, BX_COUNTOF(filePath), shader);
    bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");

    return bgfx::createShader(loadMemory(filePath));
}

bgfx::ProgramHandle loadProgram(const char* vsName, const char* fsName) {
    bgfx::ShaderHandle vs = loadShader(vsName);
    bgfx::ShaderHandle fs = loadShader(fsName);
    return bgfx::createProgram(vs, fs, true);
}
