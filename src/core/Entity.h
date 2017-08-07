#pragma once

class Entity;

class eid
{
    int16 m_value;

public:
    explicit eid(int16 value = int16(invalid()))
            : m_value(value) {}
    explicit operator int16() const { return m_value; }

    bool operator<(const eid& other) const { return m_value < other.m_value; }
    bool operator==(const eid& other) const { return m_value == other.m_value; }

    bool isValid() const { return m_value != int16(invalid()); }

    operator const Entity&() const { return get(); }
    operator Entity&() { return get(); }

    Entity&       get();
    const Entity& get() const { return const_cast<eid*>(this)->get(); }

    static eid invalid() { return eid(-1); }
};

class Entity : public dynamix::object
{
    eid         m_id;
    std::string m_name;

public:
    // TODO: make private?
    Entity(eid id = eid::invalid(), const std::string& name = "")
            : m_id(id)
            , m_name(name) {}

    const eid id() const { return m_id; }
    eid       id() { return m_id; }

    operator const eid&() const { return m_id; }
    operator eid&() { return m_id; }

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }

    HAPI void addMixin(const char* mixin);
    HAPI void remMixin(const char* mixin);

    static Entity& cast_to_entity(void* in) {
        return static_cast<Entity&>(*::dynamix::object_of(in));
    }
    static const Entity& cast_to_entity(const void* in) {
        return static_cast<const Entity&>(*::dynamix::object_of(in));
    }
};

#define ha_this Entity::cast_to_entity(this)

class HAPI EntityManager : public Singleton<EntityManager>
{
    HA_SINGLETON(EntityManager);
    EntityManager()
            : Singleton(this) {}
    friend class Application;

private:
    int16 m_curr_id = 0;

    std::map<eid, Entity> m_entities;

public:
    eid newEntityId(const std::string& in_name = std::string()) {
        std::string name = in_name;
        if(name.empty())
            name = "entity_" + std::to_string(m_curr_id);

        auto it = m_entities.emplace(eid(m_curr_id), Entity(eid(m_curr_id), name));
        it.first->second.addMixin("transform");
        it.first->second.addMixin("hierarchical");
        return eid(m_curr_id++);
    }

    Entity& newEntity(const std::string& in_name = std::string()) {
        return getById(newEntityId(in_name));
    }

    Entity& getById(eid id) { return m_entities[id]; }

    auto& getEntities() { return m_entities; }
};

inline Entity& eid::get() {
    hassert(isValid());
    return EntityManager::get().getById(*this);
}
