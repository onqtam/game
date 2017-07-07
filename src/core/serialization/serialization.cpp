#include "serialization.h"

HAPI void serialize(int data, JsonData& out) { auto res = std::to_string(data); out.append(res.c_str(), res.length()); }
HAPI void deserialize(int& data, const sajson::value& val) { data = val.get_integer_value(); }

HAPI void serialize(float data, JsonData& out) { auto res = std::to_string(data); out.append(res.c_str(), res.length()); }
HAPI void deserialize(float& data, const sajson::value& val) { data = float(val.get_double_value()); }

HAPI void serialize(double data, JsonData& out) { auto res = std::to_string(data); out.append(res.c_str(), res.length()); }
HAPI void deserialize(double& data, const sajson::value& val) { data = val.get_double_value(); }

HAPI void serialize(bool data, JsonData& out) { out.append(data ? "true" : "false", data ? 4 : 5); }
HAPI void deserialize(bool& data, const sajson::value& val) { data = (val.get_type() == sajson::TYPE_TRUE); }

HAPI void serialize(const glm::quat& data, JsonData& out) {
    out.startArray();
    serialize(data.x, out);
    out.addComma();
    serialize(data.y, out);
    out.addComma();
    serialize(data.z, out);
    out.addComma();
    serialize(data.w, out);
    out.endArray();
}
HAPI void deserialize(glm::quat& data, const sajson::value& val) {
    deserialize(data.x, val.get_array_element(0));
    deserialize(data.y, val.get_array_element(1));
    deserialize(data.z, val.get_array_element(2));
    deserialize(data.w, val.get_array_element(3));
}

HAPI void serialize(const tinygizmo::rigid_transform& data, JsonData& out) {
    out.startArray();
    serialize((glm::vec3&)data.position, out);
    out.addComma();
    serialize((glm::vec3&)data.scale, out);
    out.addComma();
    serialize((glm::vec4&)data.orientation, out);
    out.endArray();
}

HAPI void deserialize(tinygizmo::rigid_transform& data, const sajson::value& val) {
    deserialize((glm::vec3&)data.position, val.get_array_element(0));
    deserialize((glm::vec3&)data.scale, val.get_array_element(0));
    deserialize((glm::vec4&)data.orientation, val.get_array_element(0));
}

HAPI void serialize(eid data, JsonData& out) { serialize(int(data), out); }
HAPI void deserialize(eid& data, const sajson::value& val) { data = eid(val.get_integer_value()); }

HAPI void serialize(MeshHandle data, JsonData& out) { serialize(*reinterpret_cast<int*>(&data), out); }
HAPI void deserialize(MeshHandle& data, const sajson::value& val) { data = MeshHandle(int16(val.get_integer_value())); }

HAPI void serialize(ShaderHandle data, JsonData& out) { serialize(*reinterpret_cast<int*>(&data), out); }
HAPI void deserialize(ShaderHandle& data, const sajson::value& val) { data = ShaderHandle(int16(val.get_integer_value())); }
