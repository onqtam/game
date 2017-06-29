#pragma once

class JsonData
{
    std::vector<char> m_data;

public:
    size_t             size() const { return m_data.size(); }
    std::vector<char>& data() { return m_data; }
    void reserve(size_t size) { m_data.reserve(size); }

    sajson::document parse() {
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
    void append(const char* text, size_t len) {
        const size_t old_size = m_data.size();
        const size_t sum      = old_size + len;
        if(sum > m_data.capacity()) {
            if(sum < m_data.capacity() * 2)
                m_data.reserve(m_data.capacity() * 2);
            else
                m_data.reserve(sum);
        }
        m_data.resize(sum);
        memcpy(m_data.data() + old_size, text, len);
    }

    template <size_t N>
    void             append(const char (&text)[N]) {
        hassert(text[N - 1] == '\0');
        append(text, N - 1);
    }
};
