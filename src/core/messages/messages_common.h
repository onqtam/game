#pragma once

HA_SUPPRESS_WARNINGS

// common
HA_CONST_MULTI_MSG_2(common, void, serialize_mixins, cstr, concrete_mixin, JsonData&, out)
HA_MULTI_MSG_1(common, void, deserialize_mixins, const sajson::value&, in)
typedef std::vector<std::pair<const mixin_type_info*, void (*)(Object&)>> imgui_binding_callbacks;
HA_MULTI_MSG_1(common, void, get_imgui_binding_callbacks_from_mixins, imgui_binding_callbacks&, cbs)

// parental
DYNAMIX_EXPORTED_CONST_MESSAGE_0_OVERLOAD(HAPI, get_const_parent, const_oid, get_parent)
DYNAMIX_EXPORTED_MESSAGE_0_OVERLOAD(HAPI, get_non_const_parent, oid, get_parent)
DYNAMIX_EXPORTED_CONST_MESSAGE_0_OVERLOAD(HAPI, get_const_children, const std::vector<const_oid>&, get_children)
DYNAMIX_EXPORTED_MESSAGE_0_OVERLOAD(HAPI, get_non_const_children, std::vector<oid>&, get_children)
DYNAMIX_EXPORTED_MESSAGE_1(HAPI, void, set_parent, oid, _parent)

#define Interface_parental                                                                         \
    get_const_parent_msg& get_non_const_parent_msg& get_const_children_msg&                        \
            get_non_const_children_msg& set_parent_msg

// other
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_0(HAPI, void, no_gizmo) // used as a fact

HA_SUPPRESS_WARNINGS_END
