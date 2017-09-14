#pragma once

class JsonData
{
    std::vector<char> m_data;

public:
    JsonData(size_t reserve_size = 0) { m_data.reserve(reserve_size); }
    JsonData(const std::vector<char>& data)
            : m_data(data) {}
    JsonData(std::vector<char>&& data)
            : m_data(std::move(data)) {}

    size_t                   size() const { return m_data.size(); }
    std::vector<char>&       data() { return m_data; }
    const std::vector<char>& data() const { return m_data; }
    void                     reserve(size_t size) { m_data.reserve(size); }
    void                     clear() { m_data.clear(); }

    sajson::document parse() const {
        return sajson::parse(sajson::dynamic_allocation(),
                             sajson::string(m_data.data(), m_data.size()));
    }

    void addComma() { m_data.push_back(','); }
    void addNull() { m_data.push_back('\0'); }

    void startObject() { m_data.push_back('{'); }
    void startArray() { m_data.push_back('['); }

    void endObject() {
        if(m_data.back() == ',')
            m_data.back() = '}';
        else
            m_data.push_back('}');
    }

    void endArray() {
        if(m_data.back() == ',')
            m_data.back() = ']';
        else
            m_data.push_back(']');
    }

    // len should NOT include the null terminating character
    void append(cstr text, size_t len) { m_data.insert(m_data.end(), text, text + len); }

    template <size_t N>
    void append(const char (&text)[N]) {
        hassert(text[N - 1] == '\0');
        append(text, N - 1);
    }
};
