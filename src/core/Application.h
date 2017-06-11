#pragma once

#include "ObjectManager.h"

class Application
{
    HARDLY_SCOPED_SINGLETON(Application, int _main_(int, char**));

    ObjectManager m_objectManager;

public:
    void init();
    void update();
};
