#pragma once

#include "core/serialization/JsonData.h"

HA_SUPPRESS_WARNINGS

#include <tinygizmo/tiny-gizmo.hpp>

// common
DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_1(HAPI, void, serialize, JsonData&, out)
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_1(HAPI, void, deserialize, const sajson::value&, in)
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_0(HAPI, void, imgui_bind_properties)

// transform
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_pos, const glm::vec3&, pos)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_scl, const glm::vec3&, scl)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_rot, const glm::quat&, rot)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const glm::vec3&, get_pos)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const glm::vec3&, get_scl)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const glm::quat&, get_rot)

DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, glm::mat4, get_model_transform)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, move, const glm::vec3&, pos)

#define Interface_transform                                                                        \
    set_pos_msg& set_scl_msg& set_rot_msg& get_pos_msg& get_scl_msg& get_rot_msg&                  \
            get_model_transform_msg& move_msg

// hierarchical
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, eid, get_parent)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const std::vector<eid>&, get_children)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_parent, eid, _parent)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, add_child, eid, child)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, remove_child, eid, child)

#define Interface_hierarchical                                                                     \
    get_parent_msg& get_children_msg& set_parent_msg& add_child_msg& remove_child_msg

// selected
DYNAMIX_EXPORTED_MESSAGE_0(HAPI, tinygizmo::rigid_transform&, get_gizmo_transform)
DYNAMIX_EXPORTED_CONST_MULTICAST_MESSAGE_0(HAPI, void, no_gizmo) // for simulating a mixin fact - see dynamix roadmap

HA_SUPPRESS_WARNINGS_END
