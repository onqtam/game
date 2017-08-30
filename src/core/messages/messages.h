#pragma once

HA_SUPPRESS_WARNINGS

// common
HA_CONST_MULTI_MSG_2(common, void, serialize_mixins, cstr, concrete_mixin, JsonData&, out)
HA_MULTI_MSG_1(common, void, deserialize_mixins, const sajson::value&, in)
HA_MULTI_MSG_3(common, void, set_attribute_mixins, cstr, mixin, cstr, attr, const sajson::value&,
               in)
HA_MULTI_MSG_0(common, void, imgui_bind_attributes_mixins)

// transform
HA_MSG_1(tr, void, set_pos, const glm::vec3&, pos)
HA_MSG_1(tr, void, set_scl, const glm::vec3&, scl)
HA_MSG_1(tr, void, set_rot, const glm::quat&, rot)
HA_CONST_MSG_0(tr, const glm::vec3&, get_pos)
HA_CONST_MSG_0(tr, const glm::vec3&, get_scl)
HA_CONST_MSG_0(tr, const glm::quat&, get_rot)
HA_MSG_1(tr, void, set_transform, const transform&, in)
HA_CONST_MSG_0(tr, transform, get_transform)
HA_CONST_MSG_0(tr, glm::mat4, get_transform_mat)
HA_MSG_1(tr, void, move, const glm::vec3&, pos)

#define Interface_transform                                                                        \
    tr::set_pos_msg& tr::set_scl_msg& tr::set_rot_msg& tr::get_pos_msg& tr::get_scl_msg&           \
            tr::get_rot_msg& tr::set_transform_msg& tr::get_transform_msg&                         \
                    tr::get_transform_mat_msg& tr::move_msg

// parental
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, oid, get_parent)
DYNAMIX_EXPORTED_CONST_MESSAGE_0(HAPI, const std::vector<oid>&, get_children)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_parent, oid, _parent)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, add_child, oid, child)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, remove_child, oid, child)

#define Interface_hierarchical                                                                     \
    get_parent_msg& get_children_msg& set_parent_msg& add_child_msg& remove_child_msg

// selected
HA_CONST_MULTI_MSG_0(sel, void, no_gizmo) // for simulating a mixin fact - see dynamix roadmap
HA_MSG_0(sel, transform&, get_transform_on_gizmo_start)

HA_SUPPRESS_WARNINGS_END
