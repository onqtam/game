#pragma once

#include "core/rendering/GraphicsHelpers.h"

// helpers for the counting of serialization routines
#define serialize_c_impl(in) serialize
#define serialize_c serialize_c_impl(__COUNTER__)
const int serialize_definitions_counter_start = __COUNTER__;

HAPI void serialize_c(char data, JsonData& out);
HAPI void deserialize(char& data, const sajson::value& val);

HAPI void serialize_c(int data, JsonData& out);
HAPI void deserialize(int& data, const sajson::value& val);

HAPI void serialize_c(size_t data, JsonData& out);
HAPI void deserialize(size_t& data, const sajson::value& val);

HAPI void serialize_c(float data, JsonData& out);
HAPI void deserialize(float& data, const sajson::value& val);

HAPI void serialize_c(double data, JsonData& out);
HAPI void deserialize(double& data, const sajson::value& val);

HAPI void serialize_c(bool data, JsonData& out);
HAPI void deserialize(bool& data, const sajson::value& val);

HAPI void serialize_c(const std::string& data, JsonData& out);
HAPI void deserialize(std::string& data, const sajson::value& val);

template <typename T, std::enable_if_t<yama::is_yama<T>::value, int> = 0>
void serialize_c(const T& data, JsonData& out) {
    out.startArray();
    for(auto elem : data) {
        serialize(elem, out);
        out.addComma();
    }
    out.endArray();
}

template <typename T, std::enable_if_t<yama::is_yama<T>::value, int> = 0>
void deserialize(T& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_ARRAY);
    hassert(T::value_count == val.get_length());
    for(size_t i = 0; i < T::value_count; ++i)
        deserialize(data[i], val.get_array_element(i));
}

HAPI void serialize_c(const transform& data, JsonData& out);
HAPI void deserialize(transform& data, const sajson::value& val);

HAPI void serialize_c(oid data, JsonData& out);
HAPI void deserialize(oid& data, const sajson::value& val);

HAPI void serialize_c(const JsonData& data, JsonData& out);
HAPI void deserialize(JsonData& data, const sajson::value& val);

template <typename T>
void serialize_c(const std::vector<T>& data, JsonData& out) {
    out.startArray();
    for(const auto& elem : data) {
        serialize(elem, out);
        out.addComma();
    }
    out.endArray();
}

template <typename T>
void deserialize(std::vector<T>& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_ARRAY);
    auto len = val.get_length();
    data.clear();
    data.resize(len);
    for(size_t i = 0; i < len; ++i)
        deserialize(data[i], val.get_array_element(i));
}

template <typename T>
void serialize_c(const std::set<T>& data, JsonData& out) {
    out.startArray();
    for(const auto& elem : data) {
        serialize(elem, out);
        out.addComma();
    }
    out.endArray();
}

template <typename T>
void deserialize(std::set<T>& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_ARRAY);
    auto len = val.get_length();
    data.clear();
    for(size_t i = 0; i < len; ++i) {
        T temp;
        deserialize(temp, val.get_array_element(i));
        data.insert(temp);
    }
}

template <typename T1, typename T2>
void serialize_c(const std::pair<T1, T2>& data, JsonData& out) {
    out.startArray();
    serialize(data.first, out);
    out.addComma();
    serialize(data.second, out);
    out.endArray();
}

template <typename T1, typename T2>
void deserialize(std::pair<T1, T2>& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_ARRAY);
    hassert(val.get_length() == 2);
    deserialize(data.first, val.get_array_element(0));
    deserialize(data.second, val.get_array_element(1));
}

template <typename K, typename V>
void serialize_c(const std::map<K, V>& data, JsonData& out) {
    out.startArray();
    for(const auto& elem : data) {
        serialize(elem, out);
        out.addComma();
    }
    out.endArray();
}

template <typename K, typename V>
void deserialize(std::map<K, V>& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_ARRAY);
    auto len = val.get_length();
    data.clear();
    for(size_t i = 0; i < len; ++i) {
        std::pair<K, V> temp;
        deserialize(temp, val.get_array_element(i));
        data.insert(temp);
    }
}

template <typename... Ts>
void serialize_c(const std::variant<Ts...>& data, JsonData& out) {
    out.startArray();
    // type index
    serialize(data.index(), out);
    out.addComma();
    // value itself
    std::visit([&](auto&& val) { serialize(val, out); }, data);
    out.endArray();
}

template <typename... Ts>
void deserialize(std::variant<Ts...>& data, const sajson::value& val) {
    hassert(val.get_type() == sajson::TYPE_ARRAY);
    hassert(val.get_length() == 2);
    // get the type index
    int type_idx;
    deserialize(type_idx, val.get_array_element(0));

#ifdef _MSC_VER
    // previous implementation adapted from the runtime part of this SO answer: https://stackoverflow.com/a/9313217/3162383
    // current implementation adapted from this SO answer: https://stackoverflow.com/a/47500082/3162383
    constexpr std::array<void (*)(std::variant<Ts...>&, const sajson::value&), sizeof...(Ts)>
            funcs = {[](std::variant<Ts...>& var, const sajson::value& jsn) {
                var = Ts(); // default construct it so we can get it later
                deserialize(std::get<Ts>(var), jsn.get_array_element(1));
            }...};

    funcs[type_idx](data, val);
#else
    // until we get to gcc 8 we cannot use the above code because of this: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=47226
    int  i = 0;
    auto _ = {
            (type_idx == i++ ?
                     ((data = Ts(), deserialize(std::get<Ts>(data), val.get_array_element(1))), 0) :
                     0)...};
    ((void)_);
#endif
}

// helper for the counting of serialization routines
const int num_serialize_definitions = __COUNTER__ - serialize_definitions_counter_start - 1;
#undef serialize_c
#undef serialize_c_impl

// =================================================================================================
// ==  UNTESTED ====================================================================================
// =================================================================================================

HAPI void serialize(ShaderHandle data, JsonData& out);
HAPI void deserialize(ShaderHandle& data, const sajson::value& val);

// =================================================================================================
// ==  OID COLLECTION ==============================================================================
// =================================================================================================

template <typename T>
void gather_oids(T&, std::vector<const_oid*>&) {}

inline void gather_oids(const_oid& in, std::vector<const_oid*>& out) { out.push_back(&in); }
inline void gather_oids(oid& in, std::vector<const_oid*>& out) { out.push_back(&in); }

template <typename T>
void gather_oids(std::vector<T>& data, std::vector<const_oid*>& v) {
    for(auto& curr : data)
        gather_oids(curr, v);
}

template <typename K, typename V>
void gather_oids(std::map<K, V>& data, std::vector<const_oid*>& v) {
    for(auto& curr : data)
        gather_oids(curr.second, v);
}
