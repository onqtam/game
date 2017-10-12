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

current_type = ""
attributes = {}
types = {}

for line in header:
    words = line.split()
    if len(words) < 1:
        continue
    
    # start a type - also skips forward declarations
    if words[0] in {"struct", "class"} and current_type == "" and len(words) > 1 and ";" not in line:
        if words[1] in ["HAPI", "HA_EMPTY_BASE"] and len(words) > 2:
            current_type = words[2]
        else:
            current_type = words[1]
        types[current_type] = {"attributes" : attributes, "fields" : []}
        # reset attributes for the following fields
        attributes = {}
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
        types[current_type]["fields"].append({"name" : field, "attributes" : attributes})
        # reset attributes for the following fields
        attributes = {}
    # detect field or type attributes
    if words[0].startswith("REFL_ATTRIBUTES("):
        # split the contents between the 2 most outer brackets
        attributes_list = line.partition('(')[-1].rpartition(')')[0].split(",")
        # remove the whitespace in each attribute
        for index, val in enumerate(attributes_list): attributes_list[index] = val.strip()
        # stuff the attributes in a dict
        for attribute in attributes_list:
            if attribute.startswith("tag::"):
                attributes["tag"] = attribute
            elif attribute.startswith("REFL_CALLBACK"):
                attributes["callback"] = attribute.partition('(')[-1].rpartition(')')[0]
            else:
                attributes[attribute] = True

header.close()

# always generate the header - even if no functions are actually generated for the current source
# otherwise the build system will always run the custom commands for each header that haven't generated the output
code = '#pragma once\n'
code += 'HA_MSVC_SUPPRESS_WARNING(4100)\n'
code += 'HA_MSVC_SUPPRESS_WARNING(4189)\n'
code += 'HA_GCC_SUPPRESS_WARNING("-Wunused-variable")\n'
code += 'HA_GCC_SUPPRESS_WARNING("-Wunused-parameter")\n'
code += 'HA_CLANG_SUPPRESS_WARNING("-Wunused-variable")\n'
code += 'HA_CLANG_SUPPRESS_WARNING("-Wunused-parameter")\n\n'

for type in types:
    # do not continue if empty
    if not types[type]["fields"] and "REFL_NO_SKIP" not in types[type]["attributes"]:
        continue
    inline = 'inline '
    if "REFL_NO_INLINE" in types[type]["attributes"]:
        inline = ''
    
    code += strln('%svoid serialize(const %s& src, JsonData& out) {' % (inline, type))
    code += strln('out.startObject();', tabs = 1)
    for field in types[type]["fields"]:
        code += strln('HA_SERIALIZE_VARIABLE("%s", src.%s);' % (field["name"], field["name"]), tabs = 1)
    code += strln('out.endObject();', tabs = 1)
    code += strln('}')
    code += strln('')
    
    code += strln('%ssize_t deserialize(%s& dest, const sajson::value& val) {' % (inline, type))
    code += strln('const size_t val_len = val.get_length();', tabs = 1)
    code += strln('size_t num_deserialized = 0;', tabs = 1)
    code += strln('for(size_t i = 0; i < val_len; ++i) {', tabs = 1)
    for field in types[type]["fields"]:
        callback = (field["attributes"]["callback"] + '(dest)') if "callback" in field["attributes"] else '((void)0)'
        code += strln('HA_DESERIALIZE_VARIABLE("%s", dest.%s, %s);' % (field["name"], field["name"], callback), tabs = 2)
    code += strln('}', tabs = 1)
    code += strln('return num_deserialized;', tabs = 1)
    code += strln('}')
    code += strln('')
    
    code += strln('%svoid gather_oids(%s& src, std::vector<const_oid*>& out) {' % (inline, type))
    for field in types[type]["fields"]:
        code += strln('gather_oids(src.%s, out);' % (field["name"]), tabs = 1)
    code += strln('}')
    code += strln('')
    
    code += strln('%scstr imgui_bind_attributes(Object& e, cstr mixin, %s& obj) {' % (inline, type))
    code += strln('const char *out = nullptr, *temp = nullptr;', tabs = 1)
    for field in types[type]["fields"]:
        tag = (', ' + field["attributes"]["tag"] + '()') if "tag" in field["attributes"] else ''
        code += strln('temp = imgui_bind_attribute(e, mixin, "%s", obj.%s%s); if(temp) out = temp;' % (field["name"], field["name"], tag), tabs = 1)
        if "callback" in field["attributes"]:
            code += strln('if(temp) ' + field["attributes"]["callback"] + '(obj);', tabs = 1)
        
    code += strln('return out;', tabs = 1)
    code += strln('}')
    code += strln('')

code += 'HA_MSVC_SUPPRESS_WARNING_END\n'
code += 'HA_MSVC_SUPPRESS_WARNING_END\n'
code += 'HA_GCC_SUPPRESS_WARNING_END\n'
code += 'HA_GCC_SUPPRESS_WARNING_END\n'
code += 'HA_CLANG_SUPPRESS_WARNING_END\n'
code += 'HA_CLANG_SUPPRESS_WARNING_END\n'

gen = open(sys.argv[2], 'w+')
gen.write(code)
gen.close()

#if len(types) > 0:
#    import CppHeaderParser # pip install CppHeaderParser
#    try:
#        parsed = CppHeaderParser.CppHeader(sys.argv[1])
#    except CppHeaderParser.CppParseError as e:
#        print(e)
#        sys.exit(1)
#    
#    for type in types:
#        # do not continue if empty
#        if not types[type]:
#            continue
#        # do not continue if not parsed by CppHeaderParser
#        if not type in parsed.classes:
#            print type + " not parsed by CppHeaderParser!"
#            continue
#        
#        num_fields = 0
#        for visibility in ["private", "public", "protected"]:
#            num_fields += len(parsed.classes[type]["properties"][visibility])
#        
#        print type + ":"
#        print "  CppHeaderParser: " + str(num_fields)
#        print "  my file parser:  " + str(len(types[type]))
