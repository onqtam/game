#pragma once

#include "core/GraphicsHelpers.h"

struct renderPart
{
    MeshHandle   mesh;
    GeomHandle   geom;
    ShaderHandle shader;
    glm::mat4 transform;
};

HA_SUPPRESS_WARNINGS

HA_CONST_MULTI_MSG_1(rend, void, get_rendering_parts, std::vector<renderPart>&, out)
HA_CONST_MSG_0(rend, AABB, get_aabb)

HA_SUPPRESS_WARNINGS_END
