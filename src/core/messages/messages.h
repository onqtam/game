#pragma once

#include "core/serialization/JsonData.h"
#include "core/InputEvent.h"

HA_SUPPRESS_WARNINGS

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(HAPI, void, trace, std::ostream&, o)

DYNAMIX_EXPORTED_MULTICAST_MESSAGE_0(HAPI, void, imgui_bind_properties)

DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(HAPI, void, serialize, JsonData&, out)
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_1(HAPI, void, deserialize, const sajson::value&, in)

DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, process_event, const InputEvent&, ev)

// common
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_pos, const glm::vec3&, pos)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const glm::vec3&, get_pos)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, move, const glm::vec3&, pos)

// hierarchical
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, eid, get_parent)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const std::vector<eid>&, get_children)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_parent, eid, _parent)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, add_child, eid, child)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, remove_child, eid, child)

HA_SUPPRESS_WARNINGS_END
