#pragma once

#include <cstring>

struct c_string
{
    const char* data;
    c_string(const char* in = nullptr)
            : data(in) {}
    bool operator<(const c_string& other) const { return strcmp(data, other.data) < 0; }
    operator const char*() const { return data; }
    size_t length() { return strlen(data); }
};
