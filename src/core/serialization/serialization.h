#pragma once

#include "JsonData.h"

#include "core/GraphicsHelpers.h"

// helpers for the counting of serialization routines
#define serialize_c_impl(in) serialize
#define serialize_c serialize_c_impl(__COUNTER__)
const int serialize_definitions_counter_start = __COUNTER__;

HAPI void serialize_c(char data, JsonData& out);
HAPI void deserialize(char& data, const sajson::value& val);

HAPI void serialize_c(int data, JsonData& out);
HAPI void deserialize(int& data, const sajson::value& val);

HAPI void serialize_c(float data, JsonData& out);
HAPI void deserialize(float& data, const sajson::value& val);

HAPI void serialize_c(double data, JsonData& out);
HAPI void deserialize(double& data, const sajson::value& val);

HAPI void serialize_c(bool data, JsonData& out);
HAPI void deserialize(bool& data, const sajson::value& val);

HAPI void serialize_c(const std::string& data, JsonData& out);
HAPI void deserialize(std::string& data, const sajson::value& val);

template <int S, typename T>
void serialize_c(const glm::vec<S, T>& data, JsonData& out) {
    out.startArray();
    for(int i = 0; i < S; ++i) {
        serialize(data[i], out);
        out.addComma();
    }
    out.endArray();
}

template <int S, typename T>
void deserialize(glm::vec<S, T>& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_ARRAY);
    hassert(S == val.get_length());
    for(size_t i = 0; i < S; ++i)
        deserialize(data[glm::length_t(i)], val.get_array_element(i));
}

HAPI void serialize_c(const glm::quat& data, JsonData& out);
HAPI void deserialize(glm::quat& data, const sajson::value& val);

HAPI void serialize_c(eid data, JsonData& out);
HAPI void deserialize(eid& data, const sajson::value& val);

HAPI void serialize_c(MeshHandle data, JsonData& out);
HAPI void deserialize(MeshHandle& data, const sajson::value& val);

HAPI void serialize_c(ShaderHandle data, JsonData& out);
HAPI void deserialize(ShaderHandle& data, const sajson::value& val);

template <typename T>
void serialize_c(const std::vector<T>& data, JsonData& out) {
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
#undef serialize_c
#undef serialize_c_impl
