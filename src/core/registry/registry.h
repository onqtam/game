#pragma once

#include "core/serialization/JsonData.h"

#include "core/PagedMixinAllocator.h"

// =================================================================================================
// ==  MIXINS ======================================================================================
// =================================================================================================

typedef std::map<Object*, JsonData> ObjectJsonMap;
typedef void (*load_unload_proc)(ObjectJsonMap&);
typedef void (*mutate_proc)(Object*);
typedef void (*update_proc)(float dt);

struct MixinInfo
{
    mutate_proc      add;
    mutate_proc      remove;
    load_unload_proc load;
    load_unload_proc unload;
    update_proc      update;
};

typedef std::map<std::string, MixinInfo> MixinInfoMap;
typedef MixinInfoMap& (*get_mixins_proc)();

HA_SUPPRESS_WARNINGS
extern "C" HA_SYMBOL_EXPORT MixinInfoMap& getMixins();
HA_SUPPRESS_WARNINGS_END

int registerMixin(const char* name, MixinInfo info);

template <typename T>
struct UpdatableMixin
{
    static void call_update(float dt) {
        const auto& allocator  = PagedMixinAllocator<T>::get();
        const auto& flags      = allocator.getAllocatedFlags();
        const auto  flags_size = flags.size();

        for(size_t i = 0; i < flags_size; ++i)
            if(flags[i])
                allocator[i].update(dt);
    }
};

template <typename T>
typename std::enable_if<std::is_base_of<UpdatableMixin<T>, T>::value, update_proc>::type
getUpdateProc() {
    return UpdatableMixin<T>::call_update;
}

template <typename T>
typename std::enable_if<!std::is_base_of<UpdatableMixin<T>, T>::value, update_proc>::type
getUpdateProc() {
    return nullptr;
}

template <typename T>
load_unload_proc getLoadProc() {
    return [](ObjectJsonMap& in) {
        for(auto& curr : in) {
            dynamix::mutate(curr.first).add<T>();
            const sajson::document& doc = curr.second.parse();
            hassert(doc.is_valid());
            deserialize(*curr.first->get<T>(), doc.get_root());
        }
    };
}

template <typename T>
load_unload_proc getUnloadProc() {
    return [](ObjectJsonMap& out) {

        const auto& allocator  = PagedMixinAllocator<T>::get();
        const auto& flags      = allocator.getAllocatedFlags();
        const auto  flags_size = flags.size();

        for(size_t i = 0; i < flags_size; ++i) {
            if(flags[i]) {
                auto& mixin  = allocator[i];
                auto& entity = Object::cast_to_entity(&mixin);
                out[&entity].reserve(200); // for small mixins - just 1 allocation
                serialize(mixin, out[&entity]);
            }
        }
        for(auto& curr : out)
            dynamix::mutate(curr.first).remove<T>();
    };
}

#define HA_MESSAGES_IN_MIXIN(name)                                                                 \
    /* clang-format fix */ public:                                                                 \
    void serialize_mixins(JsonData& out) const {                                                   \
        out.append("\"" #name "\":");                                                              \
        serialize(*this, out);                                                                     \
        out.addComma();                                                                            \
    }                                                                                              \
    void deserialize_mixins(const sajson::value& in) {                                             \
        auto str = sajson::string(#name, HA_COUNT_OF(#name) - 1);                                  \
        if(in.find_object_key(str) != in.get_length())                                             \
            deserialize(*this, in.get_value_of_key(str));                                          \
    }                                                                                              \
    void set_attribute_mixins(const char* mixin, const char* attr, const sajson::value& in) {      \
        auto str = sajson::string(#name, HA_COUNT_OF(#name) - 1);                                  \
        if(in.find_object_key(str) != in.get_length()) {                                           \
            auto value = in.get_value_of_key(str);                                                 \
            hassert(value.get_length() == 1);                                                      \
            auto num_deserialized = deserialize(*this, value);                                     \
            hassert(num_deserialized == 1);                                                        \
        }                                                                                          \
    }                                                                                              \
    void imgui_bind_attributes_mixins() {                                                          \
        if(ImGui::TreeNode(#name)) {                                                               \
            imgui_bind_attributes(ha_this, #name, *this);                                          \
            ImGui::TreePop();                                                                      \
        }                                                                                          \
    }                                                                                              \
    /* clang-format fix */ private:

#ifdef HA_PLUGIN
#define HA_MIXIN_DEFINE_IN_PLUGIN_LOAD(n) getLoadProc<n>()
#define HA_MIXIN_DEFINE_IN_PLUGIN_UNLOAD(n) getUnloadProc<n>()
#else // HA_PLUGIN
#define HA_MIXIN_DEFINE_IN_PLUGIN_LOAD(n) nullptr
#define HA_MIXIN_DEFINE_IN_PLUGIN_UNLOAD(n) nullptr
#endif // HA_PLUGIN

#define HA_MIXIN_DEFINE_COMMON(n, features)                                                        \
    template <>                                                                                    \
    PagedMixinAllocator<n>* PagedMixinAllocator<n>::instance = nullptr;                            \
    DYNAMIX_DECLARE_MIXIN(n);                                                                      \
    DYNAMIX_DEFINE_MIXIN(n, (PagedMixinAllocator<n>::constructGlobalInstance()) & features)        \
    static int HA_CAT_1(_mixin_register_, n) = registerMixin(                                      \
            #n, /* force new line for format */                                                    \
            {[](Object* o) { dynamix::mutate(o).add<n>(); },                                       \
             [](Object* o) { dynamix::mutate(o).remove<n>(); }, HA_MIXIN_DEFINE_IN_PLUGIN_LOAD(n), \
             HA_MIXIN_DEFINE_IN_PLUGIN_UNLOAD(n), getUpdateProc<n>()})

#define HA_MIXIN_DEFINE(n, f)                                                                      \
    HA_MIXIN_DEFINE_COMMON(                                                                        \
            n,                                                                                     \
            common::serialize_mixins_msg& common::deserialize_mixins_msg&                          \
                    common::set_attribute_mixins_msg& common::imgui_bind_attributes_mixins_msg& f)

#define HA_MIXIN_DEFINE_WITHOUT_CODEGEN(n, f) HA_MIXIN_DEFINE_COMMON(n, f)

// =================================================================================================
// ==  GLOBALS =====================================================================================
// =================================================================================================

typedef void (*serialize_global_proc)(JsonData&);
typedef void (*deserialize_global_proc)(const sajson::value&);

struct GlobalInfo
{
    serialize_global_proc   serialize;
    deserialize_global_proc deserialize;
};

typedef std::map<std::string, GlobalInfo> GlobalInfoMap;
typedef GlobalInfoMap& (*get_globals_proc)();

HA_SUPPRESS_WARNINGS
extern "C" HA_SYMBOL_EXPORT GlobalInfoMap& getGlobals();
HA_SUPPRESS_WARNINGS_END

int registerGlobal(const char* name, GlobalInfo info);

// TODO: figure out how to escape the file - so it can be used as a json key
// perhaps using cmake? http://stackoverflow.com/questions/1706346/file-macro-manipulation-handling-at-compile-time
#define HA_GLOBAL_GEN_NAME(type, name) #type "_" HA_TOSTR(name) // "_" __FILE__

#define HA_GLOBAL_COMMON(type, name)                                                               \
    static int HA_ANONYMOUS(_global_) = registerGlobal(                                            \
            HA_GLOBAL_GEN_NAME(type, name),                                                        \
            {[](JsonData& out) { HA_SERIALIZE_VARIABLE(HA_GLOBAL_GEN_NAME(type, name), name); },   \
             [](const sajson::value& val) {                                                        \
                 deserialize(name, val.get_value_of_key(sajson::string(                            \
                                           HA_GLOBAL_GEN_NAME(type, name),                         \
                                           HA_COUNT_OF(HA_GLOBAL_GEN_NAME(type, name)) - 1)));     \
             }})

#define HA_GLOBAL(type, name)                                                                      \
    extern type name;                                                                              \
    HA_GLOBAL_COMMON(type, name);                                                                  \
    type name

#define HA_GLOBAL_STATIC(type, name)                                                               \
    static type name;                                                                              \
    HA_GLOBAL_COMMON(type, name)

#define HA_GLOBAL_MEMBER(type, class_type, name)                                                   \
    HA_GLOBAL_COMMON(type, HA_CAT_2(HA_CAT_2(class_type, ::), name));                              \
    type class_type::name

// == from here: http://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
//#define HA_GET_MACRO(_1, _2, _3, NAME, ...) NAME
//#define HA_WTF(...) HA_EXPAND(HA_GET_MACRO(__VA_ARGS__, macro_3, macro_2)(__VA_ARGS__))

// =================================================================================================
// ==  CODEGEN =====================================================================================
// =================================================================================================

#define HA_SERIALIZE_VARIABLE(key, var)                                                            \
    out.append("\"" key "\":");                                                                    \
    serialize(var, out);                                                                           \
    out.addComma()

#define HA_DESERIALIZE_VARIABLE(key, var)                                                          \
    if(strcmp(val.get_object_key(i).data(), key) == 0) {                                           \
        deserialize(var, val.get_object_value(i));                                                 \
        ++num_deserialized;                                                                        \
    }

#define HA_FRIENDS_OF_TYPE(name)                                                                   \
    friend void        serialize(const name& src, JsonData& out);                                  \
    friend size_t      deserialize(name& dest, const sajson::value& val);                          \
    friend const char* imgui_bind_attributes(Object& e, const char* mixin_name, name& obj)

// helpers that don't expand to anything - used by the type parser
#define FIELD           // indicates the start of a field definition inside of a type
#define EXPORT          // attribute - indicates that the field should be exported
#define ATTRIBUTES(...) // list attributes and tags in a comma-separated fashion using this
