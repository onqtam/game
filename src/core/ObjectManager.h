#pragma once

#include "utils/singleton.h"

class ObjectManager
{
    HARDLY_SCOPED_SINGLETON(ObjectManager, class Application);

    friend class PluginManager;

private:
    int m_curr_id = 0;

    std::map<int, dynamix::object> m_objects;
    
    int m_camera = -1;

public:
    void init();
    void update();
    int  shutdown();

    int new_object();
    dynamix::object& get_object(int id);

    // TODO: optimize this to not use std::string as key
    void addMixin(dynamix::object& obj, const char* mixin);
    void remMixin(dynamix::object& obj, const char* mixin);
};
