#pragma once

class eid
{
    int m_value;

public:
    explicit eid(int value = int(invalid()))
            : m_value(value) {}
    explicit operator int() const { return m_value; }

    //eid& operator++() {
    //    ++m_value;
    //    return *this;
    //}

    //eid operator++(int) {
    //    eid tmp(*this);
    //    operator++();
    //    return tmp;
    //}

    bool operator<(const eid& other) const { return m_value < other.m_value; }
    bool operator==(const eid& other) const { return m_value == other.m_value; }

    bool isValid() const { return m_value != -1; }

    static eid invalid() { return eid(-1); }
};

//inline std::ostream& operator<<(const eid& id, std::ostream& stream) {
//    stream << int(id);
//    return stream;
//}

class Entity : public dynamix::object
{
    eid         m_id;
    std::string m_name;
    bool        m_selected = false;

public:
    Entity(eid id = eid::invalid(), const std::string& name = "")
            : m_id(id)
            , m_name(name) {}

    eid id() const { return m_id; }

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    void select(bool selected = true) { m_selected = selected; }
    bool             selected() const { return m_selected; }

    static Entity& cast_to_entity(void* in) {
        return static_cast<Entity&>(*::dynamix::object_of(in));
    }
    static const Entity& cast_to_entity(const void* in) {
        return static_cast<const Entity&>(*::dynamix::object_of(in));
    }
};

#define ha_this Entity::cast_to_entity(this)
