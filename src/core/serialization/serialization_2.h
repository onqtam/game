#pragma once

#include "JsonData.h"

#include "utils/base64/base64.h"

HA_SUPPRESS_WARNINGS
#include <tinygizmo/tiny-gizmo.hpp>
HA_SUPPRESS_WARNINGS_END

// helpers for the counting of serialization routines
#define serialize_c_impl(in) serialize
#define serialize_c serialize_c_impl(__COUNTER__)
const int serialize_definitions_counter_start_2 = __COUNTER__;

HAPI void serialize_c(const tinygizmo::rigid_transform& data, JsonData& out);
HAPI void deserialize(tinygizmo::rigid_transform& data, const sajson::value& val);

// this is not necessary - the editor doesn't have to have this serialized, but it's
// a good example of how to use base64 to encode a struct (that is hopefully packed...)
HAPI void serialize_c(const tinygizmo::gizmo_application_state& data, JsonData& out);
HAPI void deserialize(tinygizmo::gizmo_application_state& data, const sajson::value& val);

// helper for the counting of serialization routines
const int num_serialize_definitions_2 = __COUNTER__ - serialize_definitions_counter_start_2 - 1;
#undef serialize_c
#undef serialize_c_impl
