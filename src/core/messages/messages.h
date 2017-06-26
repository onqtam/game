#pragma once

#include "core/serialization/JsonData.h"
#include "core/InputEvent.h"

HA_SUPPRESS_WARNINGS

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(HAPI, void, trace, std::ostream&, o)

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(HAPI, void, serialize, JsonData&, out)
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_1(HAPI, void, deserialize, const sajson::value&, in)

DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, process_event, const InputEvent&, ev)

DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_pos, const glm::vec3&, pos)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const glm::vec3&, get_pos)

HA_SUPPRESS_WARNINGS_END
