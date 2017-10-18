#include "serialization_common.h"
#include "serialization_2.h"
#include "utils/base64/base64.h"

#include <sstream>
#include <iomanip>

// taken from here: https://stackoverflow.com/a/33799784/3162383
static std::string escape_json(cstr s, size_t size) {
    std::ostringstream o;
    for(size_t i = 0; i < size; ++i) {
        switch(s[i]) {
            case '"': o << "\\\""; break;
            case '\\': o << "\\\\"; break;
            case '\b': o << "\\b"; break;
            case '\f': o << "\\f"; break;
            case '\n': o << "\\n"; break;
            case '\r': o << "\\r"; break;
            case '\t': o << "\\t"; break;
            default:
                if('\x00' <= s[i] && s[i] <= '\x1f') {
                    o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)s[i];
                } else {
                    o << s[i];
                }
        }
    }
    return o.str();
}

void serialize(char data, JsonData& out) {
    auto res = std::to_string(data);
    out.append(res.c_str(), res.length());
}
void deserialize(char& data, const sajson::value& val) { data = char(val.get_integer_value()); }

void serialize(int data, JsonData& out) {
    auto res = std::to_string(data);
    out.append(res.c_str(), res.length());
}
void deserialize(int& data, const sajson::value& val) { data = val.get_integer_value(); }

void serialize(size_t data, JsonData& out) {
    auto res = std::to_string(data);
    out.append(res.c_str(), res.length());
}
void deserialize(size_t& data, const sajson::value& val) { data = val.get_integer_value(); }

void serialize(float data, JsonData& out) {
    auto res = std::to_string(data);
    out.append(res.c_str(), res.length());
}
void deserialize(float& data, const sajson::value& val) { data = float(val.get_double_value()); }

void serialize(double data, JsonData& out) {
    auto res = std::to_string(data);
    out.append(res.c_str(), res.length());
}
void deserialize(double& data, const sajson::value& val) { data = val.get_double_value(); }

void serialize(bool data, JsonData& out) { out.append(data ? "true" : "false", data ? 4 : 5); }
void deserialize(bool& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_TRUE || val.get_type() == sajson::TYPE_FALSE);
    data = (val.get_type() == sajson::TYPE_TRUE);
}

void serialize(const std::string& data, JsonData& out) {
    out.append("\"");
    auto escaped_data = escape_json(data.c_str(), data.size());
    out.append(escaped_data.c_str(), escaped_data.length());
    out.append("\"");
}
void deserialize(std::string& data, const sajson::value& val) { data = val.as_cstring(); }

void serialize(const transform& data, JsonData& out) {
    out.startArray();
    serialize(data.pos, out);
    out.addComma();
    serialize(data.scl, out);
    out.addComma();
    serialize(data.rot, out);
    out.endArray();
}
void deserialize(transform& data, const sajson::value& val) {
    deserialize(data.pos, val.get_array_element(0));
    deserialize(data.scl, val.get_array_element(1));
    deserialize(data.rot, val.get_array_element(2));
}

void serialize(oid data, JsonData& out) { serialize(int16(data), out); }
void deserialize(oid& data, const sajson::value& val) {
    data = oid(int16(val.get_integer_value()));
}

void serialize(const JsonData& data, JsonData& out) {
    out.append("\"");
    auto escaped_data = escape_json(data.data().data(), data.size());
    out.append(escaped_data.c_str(), escaped_data.length());
    out.append("\"");
}
void deserialize(JsonData& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_STRING);
    cstr str = val.as_cstring();
    auto len = val.get_string_length();
    data.data().resize(len);
    std::copy(str, str + len, data.data().begin());
    data.addNull();
}

// serialization_2.h

void serialize(const tinygizmo::rigid_transform& data, JsonData& out) {
    out.startArray();
    serialize((const yama::vector3&)data.position, out);
    out.addComma();
    serialize((const yama::vector3&)data.scale, out);
    out.addComma();
    serialize((const yama::vector4&)data.orientation, out);
    out.endArray();
}

void deserialize(tinygizmo::rigid_transform& data, const sajson::value& val) {
    deserialize((yama::vector3&)data.position, val.get_array_element(0));
    deserialize((yama::vector3&)data.scale, val.get_array_element(1));
    deserialize((yama::vector4&)data.orientation, val.get_array_element(2));
}

void serialize(const tinygizmo::gizmo_application_state& data, JsonData& out) {
    auto max_encode_size =
            base64::getEncodedBufferMaxSize(sizeof(tinygizmo::gizmo_application_state));
    std::vector<uint8> buff(max_encode_size);
    base64::encode(reinterpret_cast<const uint8*>(&data),
                   sizeof(tinygizmo::gizmo_application_state), buff.data(), int(buff.size()));
    out.append("\"");
    out.append(reinterpret_cast<cstr>(buff.data()), buff.size());
    out.append("\"");
}

void deserialize(tinygizmo::gizmo_application_state& data, const sajson::value& val) {
    auto  str             = val.as_cstring();
    auto  max_decode_size = base64::getDecodedBufferMaxSize(int(val.get_string_length()));
    uint8 buff[sizeof(tinygizmo::gizmo_application_state) + 4];
    base64::decode(reinterpret_cast<const uint8*>(str), int(val.get_string_length()), buff,
                   max_decode_size);
    data = *reinterpret_cast<tinygizmo::gizmo_application_state*>(buff);
}

// =================================================================================================
// ==  UNTESTED ====================================================================================
// =================================================================================================

void serialize(ShaderHandle data, JsonData& out) {
    serialize(*reinterpret_cast<int16*>(&data), out);
}
void deserialize(ShaderHandle& data, const sajson::value& val) {
    data = ShaderMan::get().getHandleFromIndex_UNSAFE(int16(val.get_integer_value()));
}
