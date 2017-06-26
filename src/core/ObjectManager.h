#pragma once

class HAPI ObjectManager
{
    HARDLY_SCOPED_SINGLETON(ObjectManager, class Application);
    
    friend class PluginManager;

private:
    int m_curr_id = 0;

    std::map<int, Entity> m_objects;
    
    int m_camera = -1;
    
    void init();
    void update();
    int  shutdown();

public:
    
    int new_object_id();
    Entity& new_object();
    Entity& get_object(int id);

    // TODO: optimize this to not use std::string as key
    void addMixin(Entity& obj, const char* mixin);
    void remMixin(Entity& obj, const char* mixin);
};
