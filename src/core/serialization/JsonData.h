#pragma once

#include <vector>

#include "utils/suppress_warnings.h"

HARDLY_SUPPRESS_WARNINGS
#include <sajson/include/sajson.h>
#include <ppk_assert/ppk_assert.h>
HARDLY_SUPPRESS_WARNINGS_END

class JsonData
{
    std::vector<char> data;

public:
    size_t             getSize() const { return data.size(); }
    std::vector<char>& getData() { return data; }
    void reserve(size_t size) { data.reserve(size); }

    sajson::document parse() { return sajson::parse(sajson::dynamic_allocation(), sajson::string(data.data(), data.size())); }

    void addComma() { data.push_back(','); }
    void addNull() { data.push_back('\0'); }

    void startObject() { data.push_back('{'); }
    void startArray() { data.push_back('['); }

    void endObject() {
        if(data.back() == ',')
            data.back() = '}';
        else
            data.push_back('}');
    }

    void endArray() {
        if(data.back() == ',')
            data.back() = ']';
        else
            data.push_back(']');
    }

    // len should NOT include the null terminating character
    void append(const char* text, size_t len) {
        const size_t old_size = data.size();
        data.resize(old_size + len);
        memcpy(data.data() + old_size, text, len);
    }

    template <size_t N>
    void             append(const char (&text)[N]) {
        PPK_ASSERT(text[N - 1] == '\0');
        append(text, N - 1);
    }
};
