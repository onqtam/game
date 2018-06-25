#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include "parser.hpp"

#include "mustache.hpp"

using namespace std;

using namespace kainjow;

string no_colons(string name) {
    replace(name.begin(), name.end(), ':', '_');
    return name;
}

bool has_none_of(const set<string>& attributes, const set<string>& bad_ones) {
    for(auto& curr : bad_ones)
        if(attributes.count(curr))
            return false;
    return true;
}

int main(int argc, char** argv) {
    auto types = GetTypes(argv[1], argc, argv);

    std::fstream fs(argv[2], std::fstream::out);

    mustache::mustache template_serialization(R"raw(
{{#serialization}}{{#inline}}inline {{/inline}}void serialize(const {{type}}& src, JsonData& out) {
    out.startObject();
    {{#fields}}HA_SERIALIZE_VARIABLE("{{field}}", src.{{field}});
    {{/fields}}
    out.endObject();
}

{{#inline}}inline {{/inline}}size_t deserialize({{type}}& dest, const sajson::value& val) {
    const size_t val_len = val.get_length();
    size_t num_deserialized = 0;
    for(size_t i = 0; i < val_len; ++i) { {{#fields}}
        HA_DESERIALIZE_VARIABLE("{{field}}", dest.{{field}}, ((void)0));{{/fields}}
    }
    return num_deserialized;
}{{/serialization}}{{#imgui}}

{{#inline}}inline {{/inline}}cstr imgui_bind_attributes(Object& e, cstr mixin, {{type}}& obj) {
    const char *out = nullptr, *temp = nullptr;
    {{#fields}}temp = imgui_bind_attribute(e, mixin, "{{field}}", obj.{{field}}); if(temp) out = temp;
    {{/fields}}return out;
}{{/imgui}}
)raw");

    mustache::mustache template_in_type(R"raw(public:
{{#serialization}}
friend {{#export}}HAPI {{/export}}void serialize(const {{type}}& src, JsonData& out);
friend {{#export}}HAPI {{/export}}size_t deserialize({{type}}& dest, const sajson::value& val);
{{^nomsg}}
void serialize_mixins(cstr concrete_mixin, JsonData& out) const;
void deserialize_mixins(const sajson::value& in);
{{/nomsg}}
{{/serialization}}
{{#imgui}}
friend {{#export}}HAPI {{/export}}cstr imgui_bind_attributes(Object& e, cstr mixin, {{type}}& obj);
{{^nomsg}}
void get_imgui_binding_callbacks_from_mixins(imgui_binding_callbacks& cbs);
{{/nomsg}}
{{/imgui}}
private:
)raw");

    for(auto& curr : types) {
        if(curr->GetType() != TypeBase::Type::Class)
            continue;

        auto type = static_cast<Class*>(curr.get());

        if(type->Fields.size() == 0 || type->Attributes.count("skip"))
            continue;

        mustache::data fields = mustache::data::type::list;
        for(auto& field : type->Fields) {
            if(field.Attributes.count("skip") == 0) {
                fields.push_back(mustache::data("field", field.Name));
                //auto tag = find_if(field.Attributes.begin(), field.Attributes.end(),
                //                   [](auto in) { return in.compare(0, 5, "tag::") == 0; });
                //if(tag != field.Attributes.end()) {
                //    cout << "aa";
                //}
            }
        }

        mustache::data data("fields", fields);
        data.set("type", type->GetName());
        
        if(type->Attributes.count("inline"))
            data.set("inline", mustache::data::type::bool_true);

        if(type->Attributes.count("nomsg"))
            data.set("nomsg", mustache::data::type::bool_true);

        if(type->Attributes.count("export"))
            data.set("export", mustache::data::type::bool_true);

        if(has_none_of(type->Attributes, {"imgui", "skip"}))
            data.set("serialization", mustache::data::type::bool_true);

        if(has_none_of(type->Attributes, {"serialization", "skip"}))
            data.set("imgui", mustache::data::type::bool_true);

        fs << template_serialization.render(data);

        // the file that should be included from within the type
        std::fstream ts(string(argv[2]) + "." + no_colons(type->GetName()), std::fstream::out);

        ts << template_in_type.render(data);

        ts.close();
    }

    fs.close();

    //system("pause");
}
