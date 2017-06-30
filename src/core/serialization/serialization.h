#pragma once

#include "JsonData.h"

#include "core/GraphicsHelpers.h"

// helpers for the counting of serialization routines
#define serialize_def_impl(in) serialize
#define serialize_def serialize_def_impl(__COUNTER__)
const int serialize_definitions_counter_start = __COUNTER__;

HAPI void serialize_def(int data, JsonData& out);
HAPI void serialize_def(float data, JsonData& out);
HAPI void serialize_def(double data, JsonData& out);
HAPI void serialize_def(bool data, JsonData& out);

HAPI void deserialize(int& data, const sajson::value& val);
HAPI void deserialize(float& data, const sajson::value& val);
HAPI void deserialize(double& data, const sajson::value& val);
HAPI void deserialize(bool& data, const sajson::value& val);

HAPI void serialize_def(const glm::vec3& data, JsonData& out);
HAPI void deserialize(glm::vec3& data, const sajson::value& val);
HAPI void serialize_def(const glm::quat& data, JsonData& out);
HAPI void deserialize(glm::quat& data, const sajson::value& val);

HAPI void serialize_def(eid data, JsonData& out);
HAPI void deserialize(eid& data, const sajson::value& val);

HAPI void serialize_def(MeshHandle data, JsonData& out);
HAPI void deserialize(MeshHandle& data, const sajson::value& val);

HAPI void serialize_def(ShaderHandle data, JsonData& out);
HAPI void deserialize(ShaderHandle& data, const sajson::value& val);

template <typename T>
void serialize_def(const std::vector<T>& data, JsonData& out) {
    out.startArray();
    for(const auto& elem : data) {
        serialize(elem, out);
        out.addComma();
    }
    out.endArray();
}

template <typename T>
void deserialize(std::vector<T>& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_ARRAY);
    auto len = val.get_length();
    data.resize(len);
    for(size_t i = 0; i < len; ++i)
        deserialize(data[i], val.get_array_element(i));
}

// helper for the counting of serialization routines
const int num_serialize_definitions = __COUNTER__ - serialize_definitions_counter_start - 1;
#undef serialize_def
#undef serialize_def_impl
