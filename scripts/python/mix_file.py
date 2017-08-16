import sys
import os

def strln(string, tabs = 0):
    tabs_str = ""
    for tab in range(0, tabs):
        tabs_str += "    "
    return tabs_str + str(string) + '\n'

def writeln(f, string, tabs = 0):
    f.writelines(strln(string, tabs))

# taken from here: https://stackoverflow.com/a/61031/3162383
class ordered_dict(dict):
    def __init__(self, *args, **kwargs):
        dict.__init__(self, *args, **kwargs)
        self._order = self.keys()

    def __setitem__(self, key, value):
        dict.__setitem__(self, key, value)
        if key in self._order:
            self._order.remove(key)
        self._order.append(key)

    def __delitem__(self, key):
        dict.__delitem__(self, key)
        self._order.remove(key)

    def order(self):
        return self._order[:]

    def ordered_items(self):
        return [(key,self[key]) for key in self._order]

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
    return int(hashlib.md5(text.encode('utf-8')).hexdigest()[:8], 16)

mix_name = sys.argv[1][:-4]
mix = open(sys.argv[1], 'r')
gen = open(sys.argv[2], 'w+')

#os.path.basename(sys.argv[1])[:-4]

visibility = "protected:"

includes = ['"core/registry/registry.h"', '"core/serialization/serialization.h"', '"core/imgui/imgui_stuff.h"']
aliases = {}

functions = ""

current_type = ""
types = ordered_dict()

for line in mix:
    words = line.split()
    if len(words) < 1:
        continue
    elif words[0] == "include" and len(words) > 1:
        includes.append(words[1])
    elif words[0] == "name" and len(words) > 1:
        name = words[1]
    elif words[0] == "type" and len(words) > 1:
        current_type = words[1]
        types[current_type] = {"fields" : []}
    elif words[0] == "public:" or words[0] == "private:" or words[0] == "protected:":
        visibility = words[0]
    elif words[0] == "alias" and len(words) == 3:
        aliases[words[1]] = words[2]
    elif words[0][0] != "#" and len(words) > 1:
        field = Field()
        idx = 0
        if words[idx] == "export":
            field.export = True
            idx += 1
        
        field.type = words[idx]
        field.name = words[idx + 1]
        field.hash = field.name # str(string2numeric_hash(field.name + field.type)) # temporarily disabling the use of hashes
        field.visibility = visibility
        if line.find("#") != -1:
            field.comment = line[line.find("#") + 1:]
        for word in words[idx + 2:]:
            if word[0] == "#":
                break
            if word[:8] == "default:":
                field.default = word[8:]
        types[current_type]["fields"].append(field)

writeln(gen, "#pragma once")
writeln(gen, "")

for include in includes:
    writeln(gen, "#include " + include)
writeln(gen, "")

for type in types.order():
    name_gen = type + "_gen"

    writeln(gen, "struct " + name_gen)
    writeln(gen, "{")
    writeln(gen, "HA_FRIENDS_OF_TYPE(" + name_gen + ");", tabs = 1)
    
    visibility = ""
    for field in types[type]["fields"]:
        if visibility != field.visibility:
            visibility = field.visibility
            #writeln(gen, "")
            writeln(gen, visibility)
        if field.type in aliases.keys():
            field.type = aliases[field.type]
        default_str = ""
        if field.default != "":
            default_str = " = " + field.default
        comment_str = ""
        if field.comment != "":
            comment_str = " //" + field.comment
        writeln(gen, field.type + " " + field.name + default_str + ";" + comment_str, tabs = 1)
    
    writeln(gen, "};")
    writeln(gen, "")
    
    functions += strln("")
    functions += strln("inline void serialize(const " + name_gen + "& src, JsonData& out, bool as_object = true) {")
    functions += strln("if(as_object) out.startObject();", tabs = 1)
    for field in types[type]["fields"]:
        functions += strln("HA_SERIALIZE_VARIABLE(\"" + field.hash + "\", src." + field.name + ");", tabs = 1)
    functions += strln("if(as_object) out.endObject();", tabs = 1)
    functions += strln("}")
    functions += strln("")
    
    functions += strln("inline void deserialize(" + name_gen + "& dest, const sajson::value& val) {")
    functions += strln("const size_t val_len = val.get_length();", tabs = 1)
    functions += strln("for(size_t i = 0; i < val_len; ++i) {", tabs = 1)
    for field in types[type]["fields"]:
        functions += strln("HA_DESERIALIZE_VARIABLE(\"" + field.hash + "\", dest." + field.name + ");", tabs = 2)
    functions += strln("}", tabs = 1)
    functions += strln("}")
    functions += strln("")
    
    functions += strln("inline void imgui_bind_attributes(Entity& e, const char* mixin_name, " + name_gen + "& obj) {")
    for field in types[type]["fields"]:
        functions += strln("imgui_bind_attribute(e, mixin_name, \"" + field.name + "\", obj." + field.name + ");", tabs = 1)
    functions += strln("}")

writeln(gen, "//=============================================================================")
writeln(gen, "//=============================================================================")
writeln(gen, "//=============================================================================")
gen.write(functions)

mix.close()
gen.close()
