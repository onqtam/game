#include "serialization.h"
#include "serialization_2.h"
#include "utils/base64/base64.h"

HAPI void serialize(int data, JsonData& out) {
    auto res = std::to_string(data);
    out.append(res.c_str(), res.length());
}
HAPI void deserialize(int& data, const sajson::value& val) { data = val.get_integer_value(); }

HAPI void serialize(float data, JsonData& out) {
    auto res = std::to_string(data);
    out.append(res.c_str(), res.length());
}
HAPI void deserialize(float& data, const sajson::value& val) {
    data = float(val.get_double_value());
}

HAPI void serialize(double data, JsonData& out) {
    auto res = std::to_string(data);
    out.append(res.c_str(), res.length());
}
HAPI void deserialize(double& data, const sajson::value& val) { data = val.get_double_value(); }

HAPI void serialize(bool data, JsonData& out) { out.append(data ? "true" : "false", data ? 4 : 5); }
HAPI void deserialize(bool& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_TRUE || val.get_type() == sajson::TYPE_FALSE);
    data = (val.get_type() == sajson::TYPE_TRUE);
}

HAPI void serialize(const std::string& data, JsonData& out) {
    out.append("\"", 1);
    out.append(data.c_str(), data.length());
    out.append("\"", 1);
}
HAPI void deserialize(std::string& data, const sajson::value& val) { data = val.as_cstring(); }

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
HAPI void serialize(eid data, JsonData& out) { serialize(int(data), out); }
HAPI void deserialize(eid& data, const sajson::value& val) { data = eid(val.get_integer_value()); }

HAPI void serialize(MeshHandle data, JsonData& out) {
    serialize(*reinterpret_cast<int16*>(&data), out);
}
HAPI void deserialize(MeshHandle& data, const sajson::value& val) {
    data = MeshMan::get().getHandleFromIndex_UNSAFE(uint16(val.get_integer_value()));
}

HAPI void serialize(ShaderHandle data, JsonData& out) {
    serialize(*reinterpret_cast<int16*>(&data), out);
}
HAPI void deserialize(ShaderHandle& data, const sajson::value& val) {
    data = ShaderMan::get().getHandleFromIndex_UNSAFE(uint16(val.get_integer_value()));
}

// serialization_2.h

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
    deserialize((glm::vec3&)data.scale, val.get_array_element(1));
    deserialize((glm::vec4&)data.orientation, val.get_array_element(2));
}

HAPI void serialize(const tinygizmo::gizmo_application_state& data, JsonData& out) {
    auto max_encode_size =
            base64::getEncodedBufferMaxSize(sizeof(tinygizmo::gizmo_application_state));
    std::vector<uint8> buff(max_encode_size);
    base64::encode(reinterpret_cast<const uint8*>(&data),
                   sizeof(tinygizmo::gizmo_application_state), buff.data(), int(buff.size()));
    out.append("\"", 1);
    out.append(reinterpret_cast<const char*>(buff.data()), buff.size());
    out.append("\"", 1);
}

HAPI void deserialize(tinygizmo::gizmo_application_state& data, const sajson::value& val) {
    auto  str             = val.as_cstring();
    auto  max_decode_size = base64::getDecodedBufferMaxSize(int(val.get_string_length()));
    uint8 buff[sizeof(tinygizmo::gizmo_application_state) + 4];
    base64::decode(reinterpret_cast<const uint8*>(str), int(val.get_string_length()), buff,
                   max_decode_size);
    data = *reinterpret_cast<tinygizmo::gizmo_application_state*>(buff);
}
