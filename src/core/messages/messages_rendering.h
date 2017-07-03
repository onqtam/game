#pragma once

#include "core/GraphicsHelpers.h"

HA_SUPPRESS_WARNINGS

struct renderPart
{
    MeshHandle   mesh;
    ShaderHandle shader;
    glm::mat4 transform;
};

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(HAPI, void, get_rendering_parts, std::vector<renderPart>&, out)


HA_SUPPRESS_WARNINGS_END
