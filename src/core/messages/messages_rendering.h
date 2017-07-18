#pragma once

#include "core/GraphicsHelpers.h"

HA_SUPPRESS_WARNINGS

struct renderPart
{
    MeshHandle   mesh;
    GeomHandle   geom;
    ShaderHandle shader;
    glm::mat4 transform;
};

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(HAPI, void, get_rendering_parts, std::vector<renderPart>&, out)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, AABB, get_aabb)

HA_SUPPRESS_WARNINGS_END
