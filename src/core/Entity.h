#pragma once

HARDLY_SUPPRESS_WARNINGS

#define DYNAMIX_NO_DM_THIS
#include <dynamix/dynamix.hpp>

HARDLY_SUPPRESS_WARNINGS_END

class Entity : public dynamix::object
{
    int m_id;
public:

    Entity(int in = 0) : m_id(in) {}

    int id() const { return m_id; }
    
    static Entity& cast_to_entity(void* in) { return static_cast<Entity&>(*::dynamix::object_of(in)); }
    static const Entity& cast_to_entity(const void* in) { return static_cast<const Entity&>(*::dynamix::object_of(in)); }
};

#define ha_this Entity::cast_to_entity(this)

