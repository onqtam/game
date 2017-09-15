#pragma once

template <typename T>
JsonData mixin_attr_state(cstr mixin, cstr attr, const T& data) {
    JsonData out;
    out.startObject();
    out.append("\"");
    out.append(mixin, strlen(mixin));
    out.append("\":");
    out.startObject();
    out.append("\"");
    out.append(attr, strlen(attr));
    out.append("\":");
    serialize(data, out);
    out.endObject();
    out.endObject();

    return out;
}

inline JsonData mixin_state(const Object& obj, cstr mixin) {
    JsonData out(1000);
    out.startObject();
    common::serialize_mixins(obj, mixin, out);
    out.endObject();
    return out;
}

inline JsonData object_state(const Object& obj) {
    JsonData state(1000);
    state.startObject();
    state.append("\"\":");
    serialize(obj, state);
    state.endObject();
    return state;
}

inline std::vector<std::string> mixin_names(const Object& obj) {
    std::vector<cstr> mixins;
    obj.get_mixin_names(mixins);
    std::vector<std::string> out(mixins.size());
    std::transform(mixins.begin(), mixins.end(), out.begin(), [](auto in) { return in; });
    return out;
}

HA_SUPPRESS_WARNINGS

HA_MSG_3(edit, void, add_changed_attribute, oid, e, const JsonData&, old_val, const JsonData&,
         new_val)

#define Interface_editor edit::add_changed_attribute_msg

HA_SUPPRESS_WARNINGS_END
