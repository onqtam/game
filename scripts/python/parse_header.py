import sys
import os

def strln(string, tabs = 0):
    tabs_str = ""
    for tab in range(0, tabs):
        tabs_str += "    "
    return tabs_str + str(string) + '\n'

def writeln(f, string, tabs = 0):
    f.writelines(strln(string, tabs))

header = open(sys.argv[1], 'r')

# file should start with a line containing this
#if not "REFLECT" in next(header): sys.exit(0)

current_type = ""
attributes = {}
types = {}

for line in header:
    words = line.split()
    if len(words) < 1:
        continue
    
    # start a type
    if words[0] in {"struct", "class"} and current_type == "" and len(words) > 1 and ";" not in line:
        current_type = words[1]
        types[current_type] = []
        continue
    # end a type
    if line.strip() == "};" and current_type != "":
        current_type = ""
        continue
    # detect a field - requires the "FIELD" preprocessor identifier to be used at the start of it's definition
    if current_type != "" and words[0] == "FIELD":
        find_field_name_ender = lambda line: next((i for i, ch  in enumerate(line) if ch in {"{", "=", ";"}), None)
        ind = find_field_name_ender(line)
        # if the name is on a separate line from the type - perhaps the next one
        while not ind:
            line = next(header)
            ind = find_field_name_ender(line)
        # get the last word by splitting into words the line up until the index we found
        field = line[:ind].split()[-1]
        # add the field to the current type
        types[current_type].append({"name" : field, "attributes" : attributes})
        # reset attributes for the following fields
        attributes = {}
    # detect attributes
    if current_type != "" and words[0].startswith("ATTRIBUTES("):
        # split the contents between the 2 most outer brackets
        attributes_list = line.partition('(')[-1].rpartition(')')[0].split(",")
        # remove the whitespace in each attribute
        for index, val in enumerate(attributes_list): attributes_list[index] = val.strip()
        # stuff the attributes in a dict
        for attribute in attributes_list:
            if attribute.startswith("tag::"):
                attributes["TAG"] = attribute
            else:
                attributes[attribute] = True
code = ""

for type in types:
    # do not continue if empty
    if not types[type]:
        continue
    code += strln("void serialize(const " + type + "& src, JsonData& out, bool as_object = true) {")
    code += strln("if(as_object) out.startObject();", tabs = 1)
    for field in types[type]:
        #print field["name"]
        #print field["attributes"]
        code += strln("HA_SERIALIZE_VARIABLE(\"" + field["name"] + "\", src." + field["name"] + ");", tabs = 1)
    code += strln("if(as_object) out.endObject();", tabs = 1)
    code += strln("}")
    code += strln("")
    
    code += strln("size_t deserialize(" + type + "& dest, const sajson::value& val) {")
    code += strln("const size_t val_len = val.get_length();", tabs = 1)
    code += strln("size_t num_deserialized = 0;", tabs = 1)
    code += strln("for(size_t i = 0; i < val_len; ++i) {", tabs = 1)
    for field in types[type]:
        code += strln("HA_DESERIALIZE_VARIABLE(\"" + field["name"] + "\", dest." + field["name"] + ");", tabs = 2)
    code += strln("}", tabs = 1)
    code += strln("return num_deserialized;", tabs = 1)
    code += strln("}")
    code += strln("")
    
    code += strln("const char* imgui_bind_attributes(Object& e, const char* mixin, " + type + "& obj) {")
    code += strln("const char *out = nullptr, *temp = nullptr;", tabs = 1)
    for field in types[type]:
        code += strln("temp = imgui_bind_attribute(e, mixin, \"" + field["name"] + "\", obj." + field["name"] + ((", " + field["attributes"]["TAG"]) if "TAG" in field["attributes"] else "") + "); if(temp) out = temp;", tabs = 1)
    code += strln("return out;", tabs = 1)
    code += strln("}")
    code += strln("")

if code:
    code = "#pragma once\n\n" + code
    gen = open(sys.argv[2], 'w+')
    gen.write(code)
    gen.close()

header.close()
































