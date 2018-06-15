#pragma once

class Object;
class oid;

class ATTRS(skip) const_oid
{
    friend class ObjectManager;
    friend class oid;

    int16 m_value;

public:

    typedef int16 internal_type;

    explicit const_oid(int16 value = -1)
            : m_value(value) {}
    explicit operator int16() const { return m_value; }

    const_oid(const oid& id);

    bool operator<(const const_oid& other) const { return m_value < other.m_value; }
    bool operator==(const const_oid& other) const { return m_value == other.m_value; }
    bool operator!=(const const_oid& other) const { return m_value != other.m_value; }

    explicit operator bool() const { return isValid(); }
    bool     isValid() const;

    const Object& obj() const;

    static const_oid invalid() { return const_oid(); }
};

class ATTRS(skip) oid : public const_oid
{
public:
    explicit oid(int16 value = -1)
            : const_oid(value) {}

    Object& obj() const; // returns a non-const reference to the object - intentionally

    static oid invalid() { return oid(); }
};

class Object : public dynamix::object
{
    friend class ObjectManager;

    HA_EXPORTED_FRIENDS_OF_TYPE(Object);

    oid   m_id;
    oid m_parent;
    int m_flags          = 0;
    yama::vector3 pos    = {0, 0, 0};
    yama::vector3 scl    = {1, 1, 1};
    yama::quaternion rot = {0, 0, 0, 1};
    std::vector<oid> m_children;
    std::string m_name;

    void copy_inherited_fields(const Object& other) {
        //m_id = other.m_id; // don't copy the id!
        //m_parent = other.m_parent; // TODO: figure this out
        m_flags = other.m_flags;
        pos     = other.pos;
        scl     = other.scl;
        rot     = other.rot;
        //m_children = other.m_children; // TODO: figure this out
        m_name = other.m_name;
    }

    HAPI void orphan();
    HAPI void unparent();

    explicit Object(oid id = oid::invalid(), const std::string& name = "")
            : m_id(id)
            , m_name(name) {}

public:
    HAPI ~Object();
    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

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

    const_oid id() const { return m_id; }
    oid       id() { return m_id; }

    const std::string& name() const { return m_name; }
    void               setName(const std::string& name) { m_name = name; }

    HAPI void addMixin(cstr mixin);
    HAPI void remMixin(cstr mixin);

    // transform
    void set_pos(const yama::vector3& in) { set_transform({in, scl, rot}); }
    void set_scl(const yama::vector3& in) { set_transform({pos, in, rot}); }
    void set_rot(const yama::quaternion& in) { set_transform({pos, scl, in}); }

    yama::vector3    get_pos() const { return get_transform().pos; }
    yama::vector3    get_scl() const { return get_transform().scl; }
    yama::quaternion get_rot() const { return get_transform().rot; }

    void set_transform_local(const transform& in) {
        pos = in.pos;
        scl = in.scl;
        rot = in.rot;
    }
    transform get_transform_local() const { return {pos, scl, rot}; }

    HAPI void set_transform(const transform& in);
    HAPI transform get_transform() const;

    void move_local(const yama::vector3& in) { pos += in; }

    // parental
    const_oid                     get_parent() const { return m_parent; }
    oid                           get_parent() { return m_parent; }
    std::vector<oid>&             get_children() { return m_children; }
    const std::vector<const_oid>& get_children() const {
        return reinterpret_cast<const std::vector<const_oid>&>(m_children);
    }

    HAPI void set_parent(oid parent);

    // ha_this helpers
    static Object& cast_to_object(void* in) {
        return static_cast<Object&>(*::dynamix::object_of(in));
    }
    static const Object& cast_to_object(const void* in) {
        return static_cast<const Object&>(*::dynamix::object_of(in));
    }

    HAPI static const Object& dummy();
};

#define ha_this Object::cast_to_object(this)

class HAPI ObjectManager : public Singleton<ObjectManager>
{
    HA_SINGLETON(ObjectManager);
    ObjectManager()
            : Singleton(this) {}
    friend class Application;

    ~ObjectManager();

private:
    int16 m_curr_id = 0;

    std::map<oid, Object> m_objects;

public:

    Object& create(const std::string& in_name = "object") {
        std::string name = in_name;
        name += "_" + std::to_string(m_curr_id);

        auto it = m_objects.emplace(oid(m_curr_id), Object(oid(m_curr_id), name));

        ++m_curr_id;
        return it.first->second;
    }

    HA_GCC_SUPPRESS_WARNING("-Wuseless-cast")
    Object& createFromId(oid id) {
        hassert(!has(id));
        return m_objects.emplace(id, Object(id)).first->second;
    }

    void destroy(oid id) {
        hassert(id.isValid());
        m_objects.erase(id);
    }

    bool has(oid id) const { return m_objects.count(id) > 0; }
    bool has(const_oid id) const { return m_objects.count(oid(int16(id))) > 0; }

    Object& getById(oid id) {
        hassert(has(id));
        return m_objects.at(id);
    }

    const Object& getById(const_oid id) {
        hassert(has(oid((int16)id)));
        return m_objects.at(oid((int16)id));
    }
    HA_GCC_SUPPRESS_WARNING_END

    auto& getObjects() { return m_objects; }
};

inline const_oid::const_oid(const oid& id)
        : m_value(int16(id)) {}

inline bool const_oid::isValid() const {
    return m_value != int16(invalid()) && ObjectManager::get().has(*this);
}

inline const Object& const_oid::obj() const {
    HA_GCC_SUPPRESS_WARNING("-Wuseless-cast")
    hassert(*this != oid::invalid()); // not using isValid() to not duplicate ObjectManager::has()
    HA_GCC_SUPPRESS_WARNING_END
    return ObjectManager::get().getById(*this);
}

inline Object& oid::obj() const {
    HA_GCC_SUPPRESS_WARNING("-Wuseless-cast")
    hassert(*this != oid::invalid()); // not using isValid() to not duplicate ObjectManager::has()
    HA_GCC_SUPPRESS_WARNING_END
    return ObjectManager::get().getById(*this);
}
