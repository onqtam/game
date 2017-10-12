#pragma once

HA_SUPPRESS_WARNINGS

// common
HA_CONST_MULTI_MSG_2(common, void, serialize_mixins, cstr, concrete_mixin, JsonData&, out)
HA_MULTI_MSG_1(common, void, deserialize_mixins, const sajson::value&, in)
HA_MULTI_MSG_1(common, void, gather_oids_mixins, std::vector<const_oid*>&, in)
typedef std::vector<std::pair<const mixin_type_info*, void (*)(Object&)>> imgui_binding_callbacks;
HA_MULTI_MSG_1(common, void, get_imgui_binding_callbacks_from_mixins, imgui_binding_callbacks&, cbs)

// other
DYNAMIX_EXPORTED_MULTICAST_MESSAGE_0(HAPI, void, no_gizmo) // used as a fact

HA_SUPPRESS_WARNINGS_END
