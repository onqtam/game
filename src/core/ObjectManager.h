#pragma once

class HAPI ObjectManager
{
    HA_SCOPED_SINGLETON(ObjectManager, class Application);

    friend class PluginManager;

private:
    int m_curr_id = 0;

    std::map<eid, Entity> m_objects;

    eid m_camera;

    void init();
    void update();
    int  shutdown();

public:
    eid newObjectId(const std::string& name = std::string());
    Entity& newObject(const std::string& name = std::string());
    Entity& getObject(eid id);

    // TODO: optimize this to not use std::string as key
    void addMixin(Entity& obj, const char* mixin);
    void remMixin(Entity& obj, const char* mixin);
};
