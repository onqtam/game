#pragma once

template <typename T>
JsonData mixin_attr_state(cstr mixin, cstr attr, const T& data) {
    JsonData out;
    out.startObject();
    out.addKey(mixin);
    out.startObject();
    out.addKey(attr);
    serialize(data, out);
    out.endObject();
    out.endObject();

    return out;
}

HA_SUPPRESS_WARNINGS

HA_MSG_4(edit, void, add_changed_attribute, oid, e, const JsonData&, old_val, const JsonData&,
         new_val, const std::string&, desc)

#define Interface_editor edit::add_changed_attribute_msg

HA_SUPPRESS_WARNINGS_END
