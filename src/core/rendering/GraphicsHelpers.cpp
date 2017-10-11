#include "GraphicsHelpers.h"

#include <fstream>

HA_SINGLETON_INSTANCE(ShaderMan);
HA_SINGLETON_INSTANCE(TextureMan);
HA_SINGLETON_INSTANCE(GeomMan);

struct PosColorVertex
{
    float                   x;
    float                   y;
    float                   z;
    uint32                  abgr;
};

ha_mesh createBox(float size_x, float size_y, float size_z, uint32 color) {
    // make the sizes the halfs
    size_x /= 2;
    size_y /= 2;
    size_z /= 2;

    // clang-format off
    float vertices[8][3] = {
        { size_x, size_y, size_z}, //0
        { size_x, size_y,-size_z}, //1
        { size_x,-size_y,-size_z}, //2
        {-size_x,-size_y,-size_z}, //3
        {-size_x,-size_y, size_z}, //4
        {-size_x, size_y, size_z}, //5
        {-size_x, size_y,-size_z}, //6
        { size_x,-size_y, size_z}  //7
    };
    // clang-format on

    std::vector<PosColorVertex> verts;
    verts.reserve(24);

    verts.push_back({vertices[5][0], vertices[5][1], vertices[5][2], color});
    verts.push_back({vertices[4][0], vertices[4][1], vertices[4][2], color});
    //Top
    verts.push_back({vertices[5][0], vertices[5][1], vertices[5][2], color});
    verts.push_back({vertices[0][0], vertices[0][1], vertices[0][2], color});
    //Bottom
    verts.push_back({vertices[4][0], vertices[4][1], vertices[4][2], color});
    verts.push_back({vertices[7][0], vertices[7][1], vertices[7][2], color});
    //Right
    verts.push_back({vertices[7][0], vertices[7][1], vertices[7][2], color});
    verts.push_back({vertices[0][0], vertices[0][1], vertices[0][2], color});
    //Middle Lines
    //Top Left
    verts.push_back({vertices[6][0], vertices[6][1], vertices[6][2], color});
    verts.push_back({vertices[5][0], vertices[5][1], vertices[5][2], color});
    //Top Right
    verts.push_back({vertices[1][0], vertices[1][1], vertices[1][2], color});
    verts.push_back({vertices[0][0], vertices[0][1], vertices[0][2], color});
    //Bottom Left
    verts.push_back({vertices[3][0], vertices[3][1], vertices[3][2], color});
    verts.push_back({vertices[4][0], vertices[4][1], vertices[4][2], color});
    //Bottom Right
    verts.push_back({vertices[2][0], vertices[2][1], vertices[2][2], color});
    verts.push_back({vertices[7][0], vertices[7][1], vertices[7][2], color});
    //Back Lines
    //Left
    verts.push_back({vertices[6][0], vertices[6][1], vertices[6][2], color});
    verts.push_back({vertices[3][0], vertices[3][1], vertices[3][2], color});
    //Top
    verts.push_back({vertices[6][0], vertices[6][1], vertices[6][2], color});
    verts.push_back({vertices[1][0], vertices[1][1], vertices[1][2], color});
    //Bottom
    verts.push_back({vertices[3][0], vertices[3][1], vertices[3][2], color});
    verts.push_back({vertices[2][0], vertices[2][1], vertices[2][2], color});
    //Right
    verts.push_back({vertices[2][0], vertices[2][1], vertices[2][2], color});
    verts.push_back({vertices[1][0], vertices[1][1], vertices[1][2], color});

    ha_mesh ret;
    glGenBuffers(1, &ret.vbh);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbh);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PosColorVertex) * verts.size(), verts.data(), GL_STATIC_DRAW);

    ret.count = GLsizei(verts.size());
    ret.primitiveType = GL_LINES;

    ret.bbox = AABB({-size_x / 2, -size_y / 2, -size_z / 2}, {size_x / 2, size_y / 2, size_z / 2});
    return ret;
}

ha_mesh createSolidBox(float size_x, float size_y, float size_z, uint32 color) {
    std::vector<PosColorVertex> verts;
    verts.reserve(8);

    // make the sizes the halfs
    size_x /= 2;
    size_y /= 2;
    size_z /= 2;

    verts.push_back({-size_x, size_y, size_z, color});
    verts.push_back({size_x, size_y, size_z, color});
    verts.push_back({-size_x, -size_y, size_z, color});
    verts.push_back({size_x, -size_y, size_z, color});
    verts.push_back({-size_x, size_y, -size_z, color});
    verts.push_back({size_x, size_y, -size_z, color});
    verts.push_back({-size_x, -size_y, -size_z, color});
    verts.push_back({size_x, -size_y, -size_z, color});

    ha_mesh ret;
    glGenBuffers(1, &ret.vbh);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbh);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PosColorVertex) * verts.size(), verts.data(), GL_STATIC_DRAW);

    static const uint16 inds[] = {
            0, 1, 2, 3, 7, 1, 5, 0, 4, 2, 6, 7, 4, 5,
    };

    glGenBuffers(1, &ret.ibh);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibh);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);

    ret.primitiveType = GL_TRIANGLE_STRIP;
    ret.count = sizeof(inds) / sizeof(uint16);

    ret.bbox = AABB({-size_x, -size_y, -size_z}, {size_x, size_y, size_z});
    return ret;
}

ha_mesh createGrid(int lines_x, int lines_z, float size_x, float size_z, uint32 color) {
    hassert(lines_x > 1);
    hassert(lines_z > 1);

    float step_x = size_x / (lines_x - 1);
    float step_z = size_z / (lines_z - 1);

    std::vector<PosColorVertex> verts;
    verts.reserve((lines_x + lines_z) * 2);
    for(auto x = 0; x < lines_x; ++x) {
        verts.push_back({-size_x / 2 + step_x * x, 0.f, size_z / 2, color});
        verts.push_back({-size_x / 2 + step_x * x, 0.f, -size_z / 2, color});
    }
    for(auto z = 0; z < lines_z; ++z) {
        verts.push_back({size_x / 2, 0.f, -size_z / 2 + step_z * z, color});
        verts.push_back({-size_x / 2, 0.f, -size_z / 2 + step_z * z, color});
    }

    ha_mesh ret;
    glGenBuffers(1, &ret.vbh);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbh);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PosColorVertex) * verts.size(), verts.data(), GL_STATIC_DRAW);

    ret.count = GLsizei(verts.size());
    ret.primitiveType = GL_LINES;

    ret.bbox = AABB({-size_x / 2, -((size_x + size_z) / 2) * 0.01f, -size_z / 2},
            {size_x / 2, ((size_x + size_z) / 2) * 0.01f, size_x / 2});
    return ret;
}
