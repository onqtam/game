#pragma once

#include "JsonData.h"

HAPI void serialize(int data, JsonData& out);
HAPI void serialize(float data, JsonData& out);
HAPI void serialize(double data, JsonData& out);
HAPI void serialize(bool data, JsonData& out);

HAPI void deserialize(int& data, const sajson::value& val);
HAPI void deserialize(float& data, const sajson::value& val);
HAPI void deserialize(double& data, const sajson::value& val);
HAPI void deserialize(bool& data, const sajson::value& val);

HAPI void serialize(const glm::vec3& data, JsonData& out);
HAPI void deserialize(glm::vec3& data, const sajson::value& val);

HAPI void serialize(eid data, JsonData& out);
HAPI void deserialize(eid& data, const sajson::value& val);

template <typename T>
void serialize(const std::vector<T>& data, JsonData& out) {
    out.startArray();
    for(const auto& elem : data) {
        serialize(elem, out);
        out.addComma();
    }
    out.endArray();
}

template <typename T>
void deserialize(std::vector<T>& data, const sajson::value& val) {
    PPK_ASSERT(val.get_type() == sajson::TYPE_ARRAY);
    auto len = val.get_length();
    data.resize(len);
    for(size_t i = 0; i < len; ++i)
        deserialize(data[i], val.get_array_element(i));
}
