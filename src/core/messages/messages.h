#pragma once

#include "core/serialization/JsonData.h"

HA_SUPPRESS_WARNINGS

#include <tinygizmo/tiny-gizmo.hpp>

// common
HA_CONST_MULTI_MSG_1(common, void, serialize_mixins, JsonData&, out)
HA_MULTI_MSG_1(common, void, deserialize_mixins, const sajson::value&, in)
HA_MULTI_MSG_3(common, void, set_attribute_mixins, const char*, mixin, const char*, attr,
               const sajson::value&, in)
HA_MULTI_MSG_0(common, void, imgui_bind_attributes_mixins)

// transform
HA_MSG_1(tr, void, set_pos, const glm::vec3&, pos)
HA_MSG_1(tr, void, set_scl, const glm::vec3&, scl)
HA_MSG_1(tr, void, set_rot, const glm::quat&, rot)
HA_CONST_MSG_0(tr, const glm::vec3&, get_pos)
HA_CONST_MSG_0(tr, const glm::vec3&, get_scl)
HA_CONST_MSG_0(tr, const glm::quat&, get_rot)

HA_CONST_MSG_0(tr, glm::mat4, get_model_transform)
HA_MSG_1(tr, void, move, const glm::vec3&, pos)

#define Interface_transform                                                                        \
    tr::set_pos_msg& tr::set_scl_msg& tr::set_rot_msg& tr::get_pos_msg& tr::get_scl_msg&           \
            tr::get_rot_msg& tr::get_model_transform_msg& tr::move_msg

// hierarchical
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, oid, get_parent)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const std::vector<oid>&, get_children)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_parent, oid, _parent)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, add_child, oid, child)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, remove_child, oid, child)

#define Interface_hierarchical                                                                     \
    get_parent_msg& get_children_msg& set_parent_msg& add_child_msg& remove_child_msg

// selected
HA_MSG_0(sel, tinygizmo::rigid_transform&, get_gizmo_transform)
HA_MSG_0(sel, tinygizmo::rigid_transform&, get_last_stable_gizmo_transform)
HA_CONST_MULTI_MSG_0(sel, void, no_gizmo) // for simulating a mixin fact - see dynamix roadmap

#define Interface_selected sel::get_gizmo_transform_msg& sel::get_last_stable_gizmo_transform_msg

HA_SUPPRESS_WARNINGS_END
