import sys
import os

def strln(string, tabs = 0):
    tabs_str = ""
    for tab in range(0, tabs):
        tabs_str += "    "
    return tabs_str + str(string) + '\n'

class NewLineFile(file):
    #subclass file to have a more convienient use of writeline
    def __init__(self, name, mode = 'r'):
        self = file.__init__(self, name, mode)
    def writeln(self, string, tabs = 0):
        self.writelines(strln(string, tabs))

class Field:
    def __init__(self):
        self.type = ""
        self.name = ""
        self.hash = ""
        self.export = False
        self.default = ""
        self.visibility = ""
        self.comment = ""

def string2numeric_hash(text):
    import hashlib
    return int(hashlib.md5(text).hexdigest()[:8], 16)

mix_name = sys.argv[1][:-4]
mix = NewLineFile(sys.argv[1], 'r')
gen = NewLineFile(sys.argv[2], 'w+')

#os.path.basename(sys.argv[1])[:-4]

visibility = "protected:"

includes = ['"core/registry/registry.h"', '"core/serialization/serialization.h"', '"core/imgui/imgui_stuff.h"']
aliases = {}

functions = ""

current_type = ""
types = {}

for line in mix:
    words = line.split()
    if len(words) < 1:
        continue
    elif words[0] == "include" and len(words) > 1:
        includes.append(words[1])
    elif words[0] == "name" and len(words) > 1:
        name = words[1]
    elif (words[0] == "mixin" or words[0] == "type") and len(words) > 1:
        current_type = words[1]
        types[current_type] = {"is_mixin" : (words[0] == "mixin"), "fields" : []}
    elif words[0] == "public:" or words[0] == "private:" or words[0] == "protected:":
        visibility = words[0]
    elif words[0] == "alias" and len(words) > 2:
        aliases[words[1]] = words[2]
    elif words[0][0] != "#" and len(words) > 1:
        field = Field()
        idx = 0
        if words[idx] == "export":
            field.export = True
            idx += 1
        
        field.type = words[idx]
        field.name = words[idx + 1]
        field.hash = str(string2numeric_hash(field.name + field.type))
        field.visibility = visibility
        if line.find("#") != -1:
            field.comment = line[line.find("#") + 1:]
        for word in words[idx + 2:]:
            if word[0] == "#":
                break
            if word[:8] == "default:":
                field.default = word[8:]
        types[current_type]["fields"].append(field)

gen.writeln("#pragma once")
gen.writeln("")

for include in includes:
    gen.writeln("#include " + include)
gen.writeln("")

for type in types:
    name_gen = type + "_gen"

    gen.writeln("struct " + name_gen)
    gen.writeln("{")
    if types[type]["is_mixin"]:
        gen.writeln("HA_TYPE_MIXINABLE(" + name_gen + ");", tabs = 1)
    else:
        gen.writeln("HA_TYPE_SERIALIZABLE(" + name_gen + ");", tabs = 1)
    
    visibility = ""
    for field in types[type]["fields"]:
        if visibility != field.visibility:
            visibility = field.visibility
            #gen.writeln("")
            gen.writeln(visibility)
        if field.type in aliases.keys():
            field.type = aliases[field.type]
        default_str = ""
        if field.default != "":
            default_str = " = " + field.default
        comment_str = ""
        if field.comment != "":
            comment_str = " //" + field.comment
        gen.writeln(field.type + " " + field.name + default_str + ";" + comment_str, tabs = 1)
    
    gen.writeln("};")
    gen.writeln("")
    
    functions += strln("")
    functions += strln("inline void serialize(const " + name_gen + "& src, JsonData& out) {")
    functions += strln("out.startObject();", tabs = 1)
    for field in types[type]["fields"]:
        functions += strln("HA_SERIALIZE_VARIABLE(\"" + field.hash + "\", src." + field.name + ");", tabs = 1)
    functions += strln("out.endObject();", tabs = 1)
    functions += strln("}")
    functions += strln("")
    
    functions += strln("inline void deserialize(" + name_gen + "& dest, const sajson::value& val) {")
    functions += strln("const size_t val_len = val.get_length();", tabs = 1)
    functions += strln("for(size_t i = 0; i < val_len; ++i) {", tabs = 1)
    for field in types[type]["fields"]:
        functions += strln("HA_DESERIALIZE_VARIABLE(\"" + field.hash + "\", dest." + field.name + ");", tabs = 2)
    functions += strln("}", tabs = 1)
    functions += strln("}")
    
    functions += strln("inline void imgui_bind_property(" + name_gen + "& obj) {")
    for field in types[type]["fields"]:
        functions += strln("imgui_bind_property(\"" + field.name + "\", obj." + field.name + ");", tabs = 1)
    functions += strln("}")

gen.writeln("//=============================================================================")
gen.writeln("//=============================================================================")
gen.writeln("//=============================================================================")
gen.write(functions)

mix.close()
gen.close()
