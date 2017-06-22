#pragma once

#include <map>

#include "core/serialization/JsonData.h"
#include "utils/suppress_warnings.h"
//#include "utils/doctest/doctest_proxy.h"

HARDLY_SUPPRESS_WARNINGS

#include <dynamix/dynamix.hpp>
#include <sajson/include/sajson.h>

//#include <ppk_assert.h>

HARDLY_SUPPRESS_WARNINGS_END

#include "utils/preprocessor.h"
#include "utils/visibility.h"
#include "utils/utils.h"

#include "core/PagedMixinAllocator.h"

// =================================================================================================
// ==  MIXINS ======================================================================================
// =================================================================================================

typedef std::map<dynamix::object*, JsonData> ObjectJsonMap;
typedef void (*load_unload_proc)(ObjectJsonMap&);
typedef void (*mutate_proc)(dynamix::object*);
typedef void (*update_proc)();

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

HARDLY_SUPPRESS_WARNINGS
extern "C" SYMBOL_EXPORT MixinInfoMap& getMixins();
HARDLY_SUPPRESS_WARNINGS_END

int registerMixin(const char* name, MixinInfo info);

template <typename T>
struct UpdatableMixin
{
    static void call_update() {
        const auto& allocator  = PagedMixinAllocator<T>::get();
        const auto& flags      = allocator.getAllocatedFlags();
        const auto  flags_size = flags.size();

        for(size_t i = 0; i < flags_size; ++i)
            if(flags[i])
                allocator[i].update();
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
            PPK_ASSERT(doc.is_valid());
            deserialize(*curr.first->get<T>(), doc.get_root());
        }
    };
}

template <typename T>
load_unload_proc getUnloadProc() {
    return [](ObjectJsonMap& out) {
        for(auto& instance : T::instances) {
            out[instance.first].reserve(200); // for small mixins - just 1 allocation
            serialize(*instance.second, out[instance.first]);
        }
        for(auto& curr : out)
            dynamix::mutate(curr.first).remove<T>();
    };
}

#define HARDLY_MESSAGES_IN_MIXIN(name)                                                             \
    /* clang-format fix */ public:                                                                 \
    void serialize(JsonData& out) const {                                                          \
        out.append("\"" #name "\":");                                                              \
        ::serialize(*this, out);                                                                   \
        out.addComma();                                                                            \
    }                                                                                              \
    void deserialize(const sajson::value& in) {                                                    \
        ::deserialize(*this,                                                                       \
                      in.get_value_of_key(sajson::string(#name, HARDLY_COUNT_OF(#name) - 1)));     \
    }                                                                                              \
    /* clang-format fix */ private:

#ifdef HARDLY_PLUGIN
#define HARDLY_MIXIN_IN_PLUGIN_LOAD(n) getLoadProc<n>()
#define HARDLY_MIXIN_IN_PLUGIN_UNLOAD(n) getUnloadProc<n>()
#else // HARDLY_PLUGIN
#define HARDLY_MIXIN_IN_PLUGIN_LOAD(n) nullptr
#define HARDLY_MIXIN_IN_PLUGIN_UNLOAD(n) nullptr
#endif // HARDLY_PLUGIN

#define HARDLY_MIXIN_COMMON(n, features)                                                           \
    template <>                                                                                    \
    PagedMixinAllocator<n>* PagedMixinAllocator<n>::instance = nullptr;                            \
    DYNAMIX_DECLARE_MIXIN(n);                                                                      \
    DYNAMIX_DEFINE_MIXIN(n, (PagedMixinAllocator<n>::constructGlobalInstance()) & features)        \
    static int HARDLY_CAT_1(_mixin_register_, n) =                                                 \
            registerMixin(#n, /* force new line for format */                                      \
                          {[](dynamix::object* o) { dynamix::mutate(o).add<n>(); },                \
                           [](dynamix::object* o) { dynamix::mutate(o).remove<n>(); },             \
                           HARDLY_MIXIN_IN_PLUGIN_LOAD(n), HARDLY_MIXIN_IN_PLUGIN_UNLOAD(n),       \
                           getUpdateProc<n>()})

#define HARDLY_MIXIN(n, features)                                                                  \
    PluginInstances<HARDLY_CAT_1(n, _gen)> HARDLY_CAT_1(n, _gen)::instances;                       \
    HARDLY_MIXIN_COMMON(n, serialize_msg& deserialize_msg& features);                              \
    static_assert(sizeof(n) == sizeof(HARDLY_CAT_1(n, _gen)),                                      \
                  "Mixin '" #n "' has extended the base!")

#define HARDLY_MIXIN_WITHOUT_CODEGEN(n, features) HARDLY_MIXIN_COMMON(n, features)

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

HARDLY_SUPPRESS_WARNINGS
extern "C" SYMBOL_EXPORT GlobalInfoMap& getGlobals();
HARDLY_SUPPRESS_WARNINGS_END

int registerGlobal(const char* name, GlobalInfo info);

// TODO: figure out how to escape the file - so it can be used as a json key
// perhaps using cmake? http://stackoverflow.com/questions/1706346/file-macro-manipulation-handling-at-compile-time
#define HARDLY_GLOBAL_GEN_NAME(type, name) #type "_" HARDLY_TOSTR(name) // "_" __FILE__

#define HARDLY_GLOBAL_COMMON(type, name)                                                           \
    static int HARDLY_ANONYMOUS(_global_) = registerGlobal(                                        \
            HARDLY_GLOBAL_GEN_NAME(type, name),                                                    \
            {[](JsonData& out) {                                                                   \
                 HARDLY_SERIALIZE_VARIABLE(HARDLY_GLOBAL_GEN_NAME(type, name), name);              \
             },                                                                                    \
             [](const sajson::value& val) {                                                        \
                 deserialize(name,                                                                 \
                             val.get_value_of_key(sajson::string(                                  \
                                     HARDLY_GLOBAL_GEN_NAME(type, name),                           \
                                     HARDLY_COUNT_OF(HARDLY_GLOBAL_GEN_NAME(type, name)) - 1)));   \
             }})

#define HARDLY_GLOBAL(type, name)                                                                  \
    extern type name;                                                                              \
    HARDLY_GLOBAL_COMMON(type, name);                                                              \
    type name

#define HARDLY_GLOBAL_STATIC(type, name)                                                           \
    static type name;                                                                              \
    HARDLY_GLOBAL_COMMON(type, name)

#define HARDLY_GLOBAL_MEMBER(type, class_type, name)                                               \
    HARDLY_GLOBAL_COMMON(type, HARDLY_CAT_2(HARDLY_CAT_2(class_type, ::), name));                  \
    type class_type::name

// == from here: http://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
//#define HARDLY_GET_MACRO(_1, _2, _3, NAME, ...) NAME
//#define HARDLY_WTF(...) HARDLY_EXPAND(HARDLY_GET_MACRO(__VA_ARGS__, macro_3, macro_2)(__VA_ARGS__))

// =================================================================================================
// ==  CODEGEN =====================================================================================
// =================================================================================================

#define HARDLY_SERIALIZE_VARIABLE(key, var)                                                        \
    out.append("\"" key "\":");                                                                    \
    serialize(var, out);                                                                           \
    out.addComma()

// TODO: could be reworked to compare integers in a switch instead of strcmp-ing like crazy
#define HARDLY_DESERIALIZE_VARIABLE(key, var)                                                      \
    if(strcmp(val.get_object_key(i).data(), key) == 0)                                             \
    deserialize(var, val.get_object_value(i))

template <class T>
using PluginInstances = std::vector<std::pair<dynamix::object*, T*>>;

#define HARDLY_TRACK_INSTANCES(type)                                                               \
    static PluginInstances<type> instances;                                                        \
    type() { instances.push_back(std::make_pair(dm_this, this)); }                                 \
    ~type() {                                                                                      \
        PluginInstances<type>::iterator it =                                                       \
                find(instances.begin(), instances.end(), std::make_pair(dm_this, this));           \
        if(it + 1 != instances.end())                                                              \
            *it = *(instances.end() - 1);                                                          \
        instances.pop_back();                                                                      \
    }                                                                                              \
    type(const type&) = default;                                                                   \
    type& operator=(const type&) = default

#define HARDLY_TYPE_SERIALIZABLE(name)                                                             \
    friend void serialize(const name& src, JsonData& out);                                         \
    friend void deserialize(name& dest, const sajson::value& val)

#define HARDLY_TYPE_MIXINABLE(name)                                                                \
    HARDLY_TRACK_INSTANCES(name);                                                                  \
    HARDLY_TYPE_SERIALIZABLE(name)
