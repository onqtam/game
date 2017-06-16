#pragma once

#include "utils/singleton.h"

class ObjectManager
{
    HARDLY_SCOPED_SINGLETON(ObjectManager, class Application);

    friend class PluginManager;

private:
    //std::vector<dynamix::object> m_objects;

    dynamix::object m_object;

public:
    void init();
    void update();

    // TODO: optimize this to not use std::string as key
    void addMixin(dynamix::object& obj, const char* mixin);
    void remMixin(dynamix::object& obj, const char* mixin);
};
