#include "GraphicsHelpers.h"

#include <fstream>

HA_SUPPRESS_WARNINGS

#include <bx/bx.h>
#include <bx/string.h>
#include <bx/crtimpl.h>
#include <bimg/bimg.h>
#include <bimg/decode.h>

//#include <bgfx/src/vertexdecl.h>
//=== The following functions are the ones from <bgfx/src/vertexdecl.h> but reimplemented using the C api because they aren't exported...
//Attrib::Enum idToAttrib(uint16_t id);
//AttribType::Enum idToAttribType(uint16_t id);
//int32_t read(bx::ReaderI* _reader, bgfx::VertexDecl& _decl, bx::Error* _err = NULL);

struct AttribToId
{
    bgfx_attrib attr;
    uint16_t    id;
};

static AttribToId s_attribToId[] = {
        // NOTICE:
        // Attrib must be in order how it appears in Attrib::Enum! id is
        // unique and should not be changed if new Attribs are added.
        {BGFX_ATTRIB_POSITION, 0x0001},  {BGFX_ATTRIB_NORMAL, 0x0002},
        {BGFX_ATTRIB_TANGENT, 0x0003},   {BGFX_ATTRIB_BITANGENT, 0x0004},
        {BGFX_ATTRIB_COLOR0, 0x0005},    {BGFX_ATTRIB_COLOR1, 0x0006},
        {BGFX_ATTRIB_COLOR2, 0x0018},    {BGFX_ATTRIB_COLOR3, 0x0019},
        {BGFX_ATTRIB_INDICES, 0x000e},   {BGFX_ATTRIB_WEIGHT, 0x000f},
        {BGFX_ATTRIB_TEXCOORD0, 0x0010}, {BGFX_ATTRIB_TEXCOORD1, 0x0011},
        {BGFX_ATTRIB_TEXCOORD2, 0x0012}, {BGFX_ATTRIB_TEXCOORD3, 0x0013},
        {BGFX_ATTRIB_TEXCOORD4, 0x0014}, {BGFX_ATTRIB_TEXCOORD5, 0x0015},
        {BGFX_ATTRIB_TEXCOORD6, 0x0016}, {BGFX_ATTRIB_TEXCOORD7, 0x0017},
};
BX_STATIC_ASSERT(BX_COUNTOF(s_attribToId) == BGFX_ATTRIB_COUNT);

bgfx_attrib idToAttrib(uint16_t id) {
    for(uint32_t ii = 0; ii < BX_COUNTOF(s_attribToId); ++ii) {
        if(s_attribToId[ii].id == id) {
            return s_attribToId[ii].attr;
        }
    }

    return BGFX_ATTRIB_COUNT;
}

struct AttribTypeToId
{
    bgfx_attrib_type type;
    uint16_t         id;
};

static AttribTypeToId s_attribTypeToId[] = {
        // NOTICE:
        // AttribType must be in order how it appears in AttribType::Enum!
        // id is unique and should not be changed if new AttribTypes are
        // added.
        {BGFX_ATTRIB_TYPE_UINT8, 0x0001}, {BGFX_ATTRIB_TYPE_UINT10, 0x0005},
        {BGFX_ATTRIB_TYPE_INT16, 0x0002}, {BGFX_ATTRIB_TYPE_HALF, 0x0003},
        {BGFX_ATTRIB_TYPE_FLOAT, 0x0004},
};
BX_STATIC_ASSERT(BX_COUNTOF(s_attribTypeToId) == BGFX_ATTRIB_TYPE_COUNT);

bgfx_attrib_type idToAttribType(uint16_t id) {
    for(uint32_t ii = 0; ii < BX_COUNTOF(s_attribTypeToId); ++ii) {
        if(s_attribTypeToId[ii].id == id) {
            return s_attribTypeToId[ii].type;
        }
    }

    return BGFX_ATTRIB_TYPE_COUNT;
}

int32_t read(bx::ReaderI* _reader, bgfx_vertex_decl& _decl, bx::Error* _err) {
    BX_ERROR_SCOPE(_err);

    int32_t total = 0;

    uint8_t numAttrs;
    total += bx::read(_reader, numAttrs, _err);

    uint16_t stride;
    total += bx::read(_reader, stride, _err);

    if(!_err->isOk()) {
        return total;
    }

    bgfx_vertex_decl_begin(&_decl, BGFX_RENDERER_TYPE_COUNT);

    for(uint32_t ii = 0; ii < numAttrs; ++ii) {
        uint16_t offset;
        total += bx::read(_reader, offset, _err);

        uint16_t attribId = 0;
        total += bx::read(_reader, attribId, _err);

        uint8_t num;
        total += bx::read(_reader, num, _err);

        uint16_t attribTypeId;
        total += bx::read(_reader, attribTypeId, _err);

        bool normalized;
        total += bx::read(_reader, normalized, _err);

        bool asInt;
        total += bx::read(_reader, asInt, _err);

        if(!_err->isOk()) {
            return total;
        }

        bgfx_attrib      attr = idToAttrib(attribId);
        bgfx_attrib_type type = idToAttribType(attribTypeId);
        if(BGFX_ATTRIB_COUNT != attr && BGFX_ATTRIB_TYPE_COUNT != type) {
            bgfx_vertex_decl_add(&_decl, bgfx_attrib(attr), num, bgfx_attrib_type(type), normalized,
                                 asInt);
            _decl.offset[attr] = offset;
        }
    }

    bgfx_vertex_decl_end(&_decl);
    _decl.stride = stride;

    return total;
}

//=== END OF REIMPLEMENTED VERTEX DECL READ OVERLOAD

#include <ib-compress/indexbufferdecompression.h>

#include <tinystl/allocator.h>
#include <tinystl/vector.h>
#include <tinystl/string.h>

HA_SUPPRESS_WARNINGS_END

namespace stl = tinystl;

HA_SUPPRESS_WARNINGS

bx::DefaultAllocator g_allocator;

static bx::AllocatorI* getAllocator() { return &g_allocator; }

static bx::FileReaderI* getFileReader() {
    static bx::FileReaderI* s_fileReader = BX_NEW(&g_allocator, bx::FileReader);
    return s_fileReader;
}

static void* load(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath,
                  uint32* _size) {
    if(bx::open(_reader, _filePath)) {
        uint32 size = (uint32)bx::getSize(_reader);
        void*  data = BX_ALLOC(_allocator, size);
        bx::read(_reader, data, size);
        bx::close(_reader);
        if(nullptr != _size) {
            *_size = size;
        }
        return data;
    } else {
        //DBG("Failed to open: %s.", _filePath);
    }

    if(nullptr != _size) {
        *_size = 0;
    }

    return nullptr;
}
static void unload(void* _ptr) { BX_FREE(getAllocator(), _ptr); }

const bgfx_memory* loadMemory(const char* filename) {
    std::ifstream   file(filename, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    const bgfx_memory* mem = bgfx_alloc(uint32(size + 1));
    if(file.read((char*)mem->data, size)) {
        mem->data[mem->size - 1] = '\0';
        return mem;
    }
    return nullptr;
}

bgfx_shader_handle loadShader(const char* shader) {
    char filePath[512];

    const char* shaderPath = "???";

    switch(bgfx_get_renderer_type()) {
        case BGFX_RENDERER_TYPE_NOOP:
        case BGFX_RENDERER_TYPE_DIRECT3D9: shaderPath = "shaders/dx9/"; break;
        case BGFX_RENDERER_TYPE_DIRECT3D11:
        case BGFX_RENDERER_TYPE_DIRECT3D12: shaderPath = "shaders/dx11/"; break;
        case BGFX_RENDERER_TYPE_GNM: shaderPath        = "shaders/pssl/"; break;
        case BGFX_RENDERER_TYPE_METAL: shaderPath      = "shaders/metal/"; break;
        case BGFX_RENDERER_TYPE_OPENGL: shaderPath     = "shaders/glsl/"; break;
        case BGFX_RENDERER_TYPE_OPENGLES: shaderPath   = "shaders/gles/"; break;
        case BGFX_RENDERER_TYPE_VULKAN: shaderPath     = "shaders/spirv/"; break;
        case BGFX_RENDERER_TYPE_COUNT: BX_CHECK(false, "You should not be here!"); break;
    }

    bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
    bx::strCat(filePath, BX_COUNTOF(filePath), shader);
    bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");

    return bgfx_create_shader(loadMemory(filePath));
}

bgfx_program_handle loadProgram(const char* vsName, const char* fsName) {
    bgfx_shader_handle vs = loadShader(vsName);
    bgfx_shader_handle fs = loadShader(fsName);
    return bgfx_create_program(vs, fs, true);
}

static void imageReleaseCb(void*, void* _userData) {
    bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
    bimg::imageFree(imageContainer);
}

static bgfx_texture_handle loadTexture(bx::FileReaderI* _reader, const char* _filePath,
                                       uint32 _flags, uint8_t, bgfx_texture_info* _info) {
    bgfx_texture_handle handle = {BGFX_INVALID_HANDLE};

    uint32 size;
    void*  data = load(_reader, getAllocator(), _filePath, &size);
    if(nullptr != data) {
        bimg::ImageContainer* imageContainer = bimg::imageParse(getAllocator(), data, size);

        if(nullptr != imageContainer) {
            const bgfx_memory* mem = bgfx_make_ref_release(
                    imageContainer->m_data, imageContainer->m_size, imageReleaseCb, imageContainer);
            unload(data);

            if(imageContainer->m_cubeMap) {
                handle = bgfx_create_texture_cube(
                        uint16_t(imageContainer->m_width), 1 < imageContainer->m_numMips,
                        imageContainer->m_numLayers, bgfx_texture_format(imageContainer->m_format),
                        _flags, mem);
            } else {
                handle = bgfx_create_texture_2d(
                        uint16_t(imageContainer->m_width), uint16_t(imageContainer->m_height),
                        1 < imageContainer->m_numMips, imageContainer->m_numLayers,
                        bgfx_texture_format(imageContainer->m_format), _flags, mem);
            }

            if(nullptr != _info) {
                bgfx_calc_texture_size(_info, uint16_t(imageContainer->m_width),
                                       uint16_t(imageContainer->m_height),
                                       uint16_t(imageContainer->m_depth), imageContainer->m_cubeMap,
                                       1 < imageContainer->m_numMips, imageContainer->m_numLayers,
                                       bgfx_texture_format(imageContainer->m_format));
            }
        }
    }

    return handle;
}

bgfx_texture_handle loadTexture(const char* _name, uint32 _flags, uint8_t _skip,
                                bgfx_texture_info* _info) {
    return loadTexture(getFileReader(), _name, _flags, _skip, _info);
}

struct Aabb
{
    float m_min[3];
    float m_max[3];
};

struct Obb
{ float m_mtx[16]; };

struct Sphere
{
    float m_center[3];
    float m_radius;
};

struct Primitive
{
    uint32 m_startIndex;
    uint32 m_numIndices;
    uint32 m_startVertex;
    uint32 m_numVertices;

    Sphere m_sphere;
    Aabb   m_aabb;
    Obb    m_obb;
};

typedef stl::vector<Primitive> PrimitiveArray;

struct Group
{
    Group() { reset(); }

    void reset() {
        m_vbh.idx = BGFX_INVALID_HANDLE;
        m_ibh.idx = BGFX_INVALID_HANDLE;
        m_prims.clear();
    }

    bgfx_vertex_buffer_handle m_vbh;
    bgfx_index_buffer_handle  m_ibh;
    Sphere                    m_sphere;
    Aabb                      m_aabb;
    Obb                       m_obb;
    PrimitiveArray            m_prims;
};

struct Mesh
{
    void load(bx::ReaderSeekerI* _reader) {
#define BGFX_CHUNK_MAGIC_VB BX_MAKEFOURCC('V', 'B', ' ', 0x1)
#define BGFX_CHUNK_MAGIC_IB BX_MAKEFOURCC('I', 'B', ' ', 0x0)
#define BGFX_CHUNK_MAGIC_IBC BX_MAKEFOURCC('I', 'B', 'C', 0x0)
#define BGFX_CHUNK_MAGIC_PRI BX_MAKEFOURCC('P', 'R', 'I', 0x0)

        using namespace bx;

        Group group;

        bx::AllocatorI* allocator = getAllocator();

        uint32    chunk;
        bx::Error err;
        while(4 == bx::read(_reader, chunk, &err) && err.isOk()) {
            switch(chunk) {
                case BGFX_CHUNK_MAGIC_VB: {
                    read(_reader, group.m_sphere);
                    read(_reader, group.m_aabb);
                    read(_reader, group.m_obb);

                    read(_reader, m_decl, nullptr);

                    uint16_t stride = m_decl.stride;

                    uint16_t numVertices;
                    read(_reader, numVertices);
                    const bgfx_memory* mem = bgfx_alloc(numVertices * stride);
                    read(_reader, mem->data, mem->size);

                    group.m_vbh = bgfx_create_vertex_buffer(mem, &m_decl, BGFX_BUFFER_NONE);
                } break;

                case BGFX_CHUNK_MAGIC_IB: {
                    uint32 numIndices;
                    read(_reader, numIndices);
                    const bgfx_memory* mem = bgfx_alloc(numIndices * 2);
                    read(_reader, mem->data, mem->size);
                    group.m_ibh = bgfx_create_index_buffer(mem, BGFX_BUFFER_NONE);
                } break;

                case BGFX_CHUNK_MAGIC_IBC: {
                    uint32 numIndices;
                    bx::read(_reader, numIndices);

                    const bgfx_memory* mem = bgfx_alloc(numIndices * 2);

                    uint32 compressedSize;
                    bx::read(_reader, compressedSize);

                    void* compressedIndices = BX_ALLOC(allocator, compressedSize);

                    bx::read(_reader, compressedIndices, compressedSize);

                    ReadBitstream rbs((const uint8_t*)compressedIndices, compressedSize);
                    DecompressIndexBuffer((uint16_t*)mem->data, numIndices / 3, rbs);

                    BX_FREE(allocator, compressedIndices);

                    group.m_ibh = bgfx_create_index_buffer(mem, BGFX_BUFFER_NONE);
                } break;

                case BGFX_CHUNK_MAGIC_PRI: {
                    uint16_t len;
                    read(_reader, len);

                    stl::string material;
                    material.resize(len);
                    read(_reader, const_cast<char*>(material.c_str()), len);

                    uint16_t num;
                    read(_reader, num);

                    for(uint32 ii = 0; ii < num; ++ii) {
                        read(_reader, len);

                        stl::string name;
                        name.resize(len);
                        read(_reader, const_cast<char*>(name.c_str()), len);

                        Primitive prim;
                        read(_reader, prim.m_startIndex);
                        read(_reader, prim.m_numIndices);
                        read(_reader, prim.m_startVertex);
                        read(_reader, prim.m_numVertices);
                        read(_reader, prim.m_sphere);
                        read(_reader, prim.m_aabb);
                        read(_reader, prim.m_obb);

                        group.m_prims.push_back(prim);
                    }

                    m_groups.push_back(group);
                    group.reset();
                } break;

                default: //DBG("%08x at %d", chunk, bx::skip(_reader, 0));
                    break;
            }
        }
    }

    void unload() {
        for(GroupArray::const_iterator it = m_groups.begin(), itEnd = m_groups.end(); it != itEnd;
            ++it) {
            const Group& group = *it;
            bgfx_destroy_vertex_buffer(group.m_vbh);

            if(group.m_ibh.idx != BGFX_INVALID_HANDLE)
                bgfx_destroy_index_buffer(group.m_ibh);
        }
        m_groups.clear();
    }

    void submit(uint8_t _id, bgfx_program_handle _program, const float* _mtx,
                uint64_t _state) const {
        if(BGFX_STATE_MASK == _state) {
            _state = 0 | BGFX_STATE_RGB_WRITE | BGFX_STATE_ALPHA_WRITE | BGFX_STATE_DEPTH_WRITE |
                     BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA;
        }

        bgfx_set_transform(_mtx, 1);
        bgfx_set_state(_state, 0);

        for(GroupArray::const_iterator it = m_groups.begin(), itEnd = m_groups.end(); it != itEnd;
            ++it) {
            const Group& group = *it;

            bgfx_set_index_buffer(group.m_ibh, 0, UINT32_MAX);
            bgfx_set_vertex_buffer(0, group.m_vbh, 0, UINT32_MAX);
            bgfx_submit(_id, _program, 0, it != itEnd - 1);
        }
    }

    bgfx_vertex_decl           m_decl;
    typedef stl::vector<Group> GroupArray;
    GroupArray                 m_groups;
};

static Mesh* meshLoad(bx::ReaderSeekerI* _reader) {
    Mesh* mesh = new Mesh;
    mesh->load(_reader);
    return mesh;
}

Mesh* meshLoad(const char* _filePath) {
    bx::FileReaderI* reader = getFileReader();
    if(bx::open(reader, _filePath)) {
        Mesh* mesh = meshLoad(reader);
        bx::close(reader);
        return mesh;
    }

    return nullptr;
}

void meshUnload(Mesh* _mesh) {
    _mesh->unload();
    delete _mesh;
}

void meshSubmit(const Mesh* _mesh, uint8_t _id, bgfx_program_handle _program, const float* _mtx,
                uint64_t _state) {
    _mesh->submit(_id, _program, _mtx, _state);
}

HA_SUPPRESS_WARNINGS_END

// =================================================================================================
// == END OF BGFX EXAMPLE CODE =====================================================================
// =================================================================================================

struct PosColorVertex
{
    float  x;
    float                   y;
    float                   z;
    uint32                  abgr;
    static bgfx_vertex_decl vd;
};

bgfx_vertex_decl PosColorVertex::vd = []() {
    bgfx_vertex_decl_begin(&PosColorVertex::vd, BGFX_RENDERER_TYPE_COUNT);
    bgfx_vertex_decl_add(&PosColorVertex::vd, BGFX_ATTRIB_POSITION, 3, BGFX_ATTRIB_TYPE_FLOAT,
                         false, false);
    bgfx_vertex_decl_add(&PosColorVertex::vd, BGFX_ATTRIB_COLOR0, 4, BGFX_ATTRIB_TYPE_UINT8, true,
                         false);
    bgfx_vertex_decl_end(&PosColorVertex::vd);
    return PosColorVertex::vd;
}();

ha_mesh createCube() {
    static const PosColorVertex s_cubeVertices[] = {
            {-1.0f, 1.0f, 1.0f, 0xff000000},   {1.0f, 1.0f, 1.0f, 0xff0000ff},
            {-1.0f, -1.0f, 1.0f, 0xff00ff00},  {1.0f, -1.0f, 1.0f, 0xff00ffff},
            {-1.0f, 1.0f, -1.0f, 0xffff0000},  {1.0f, 1.0f, -1.0f, 0xffff00ff},
            {-1.0f, -1.0f, -1.0f, 0xffffff00}, {1.0f, -1.0f, -1.0f, 0xffffffff},
    };

    static const uint16 s_cubeTriStrip[] = {
            0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5,
    };

    auto vbh = bgfx_create_vertex_buffer(bgfx_make_ref(s_cubeVertices, sizeof(s_cubeVertices)),
                                         &PosColorVertex::vd, BGFX_BUFFER_NONE);
    auto ibh = bgfx_create_index_buffer(bgfx_make_ref(s_cubeTriStrip, sizeof(s_cubeTriStrip)),
                                        BGFX_BUFFER_NONE);

    return {vbh, ibh, BGFX_STATE_PT_TRISTRIP};
}

ha_mesh createGrid(int lines_x, int lines_z, float size_x, float size_z, uint32 color) {
    hassert(lines_x > 1);
    hassert(lines_z > 1);

    float step_x = size_x / (lines_x - 1);
    float step_z = size_z / (lines_z - 1);

    std::vector<PosColorVertex> verts;
    verts.reserve((lines_x + lines_z) * 2);
    for(auto x = 0; x < lines_x; ++x) {
        verts.push_back({-size_x / 2 + step_x * x, 0.f, size_z / 2});
        verts.push_back({-size_x / 2 + step_x * x, 0.f, -size_z / 2});
    }
    for(auto z = 0; z < lines_z; ++z) {
        verts.push_back({size_x / 2, 0.f, -size_z / 2 + step_z * z});
        verts.push_back({-size_x / 2, 0.f, -size_z / 2 + step_z * z});
    }

    auto vbh = bgfx_create_vertex_buffer(bgfx_copy(verts.data(), verts.size() * sizeof(PosColorVertex)),
                                         &PosColorVertex::vd, BGFX_BUFFER_NONE);
    
    return {vbh, {BGFX_INVALID_HANDLE}, BGFX_STATE_PT_LINES};
}
