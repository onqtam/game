#pragma once

#include "messages.h"

template <typename T>
JsonData attr_changed_command(cstr mixin, cstr attr, const T& data) {
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

inline JsonData mixin_state_command(oid id, cstr mixin) {
    JsonData out(1000);
    out.startObject();
    common::serialize_mixins(id, mixin, out);
    out.endObject();
    return out;
}

inline std::vector<std::string> mixin_names(oid id) {
    std::vector<cstr> mixins;
    id.get().get_mixin_names(mixins);
    std::vector<std::string> out(mixins.size());
    std::transform(mixins.begin(), mixins.end(), out.begin(), [](auto in) { return in; });
    return out;
}

HA_SUPPRESS_WARNINGS

HA_MSG_3(edit, void, add_changed_attribute, oid, e, const json_buf&, old_val, const json_buf&,
         new_val)

#define Interface_editor edit::add_changed_attribute_msg

HA_SUPPRESS_WARNINGS_END
