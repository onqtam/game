#pragma once

class Object;

// TODO: rethink this - currently a const oid can be copied and non-const messages can be called on the copy - is that ok?
class oid
{
    int16 m_value;

public:
    explicit oid(int16 value = int16(invalid()))
            : m_value(value) {}
    explicit operator int16() const { return m_value; }

    bool operator<(const oid& other) const { return m_value < other.m_value; }
    bool operator==(const oid& other) const { return m_value == other.m_value; }

    bool isValid() const;

    operator const Object&() const { return get(); }
    operator Object&() { return get(); }

    Object&       get();
    const Object& get() const { return const_cast<oid*>(this)->get(); }

    static oid invalid() { return oid(-1); }
};

class Object : public dynamix::object
{
    friend HAPI void serialize(const Object& src, JsonData& out);
    friend HAPI size_t      deserialize(Object& dest, const sajson::value& val);
    friend HAPI const char* imgui_bind_attributes(Object& e, const char* mixin, Object& obj);

    oid   m_id;
    FIELD std::string m_name;
    FIELD int         m_flags = 0;

    void copy_inherited_fields(const Object& other) {
        m_id    = other.m_id;
        m_name  = other.m_name;
        m_flags = other.m_flags;
    }

public:
    // TODO: make private?
    Object(oid id = oid::invalid(), const std::string& name = "")
            : m_id(id)
            , m_name(name) {}

    // hides dynamix::object::copy()
    Object copy() const {
        Object o;
        o.copy_from(*this);
        return o;
    }
    // hides dynamix::object::copy_from()
    void copy_from(const Object& o) {
        if(this == &o)
            return;

        dynamix::object::copy_from(o);
        copy_inherited_fields(o);
    }
    // hides dynamix::object::copy_matching_from()
    void copy_matching_from(const Object& o) {
        if(this == &o)
            return;

        dynamix::object::copy_matching_from(o);
        copy_inherited_fields(o);
    }

    const oid id() const { return m_id; }
    oid       id() { return m_id; }

    operator const oid&() const { return m_id; }
    operator oid&() { return m_id; }

    const std::string& name() const { return m_name; }
    void               setName(const std::string& name) { m_name = name; }

    HAPI void addMixin(const char* mixin);
    HAPI void remMixin(const char* mixin);

    static Object& cast_to_object(void* in) {
        return static_cast<Object&>(*::dynamix::object_of(in));
    }
    static const Object& cast_to_object(const void* in) {
        return static_cast<const Object&>(*::dynamix::object_of(in));
    }
};

#define ha_this Object::cast_to_object(this)

class HAPI ObjectManager : public Singleton<ObjectManager>
{
    HA_SINGLETON(ObjectManager);
    ObjectManager()
            : Singleton(this) {}
    friend class Application;

private:
    int16 m_curr_id = 0;

    std::map<oid, Object> m_objects;

public:
    oid create(const std::string& in_name = std::string()) {
        std::string name = in_name;
        if(name.empty())
            name = "object_" + std::to_string(m_curr_id);

        auto it = m_objects.emplace(oid(m_curr_id), Object(oid(m_curr_id), name));
        it.first->second.addMixin("transform");
        it.first->second.addMixin("hierarchical");
        return oid(m_curr_id++);
    }

    oid createFromId(oid id) {
        hassert(!has(id));
        return m_objects.emplace(id, Object(id)).first->second;
    }

    void destroy(oid id) {
        hassert(id.isValid());
        m_objects.erase(id);
    }

    bool has(oid id) const { return m_objects.count(id) > 0; }

    Object& getById(oid id) {
        hassert(has(id));
        return m_objects.at(id);
    }

    auto& getObjects() { return m_objects; }
};

inline bool oid::isValid() const {
    return m_value != int16(invalid()) && ObjectManager::get().has(*this);
}

inline Object& oid::get() {
    hassert(isValid());
    return ObjectManager::get().getById(*this);
}
