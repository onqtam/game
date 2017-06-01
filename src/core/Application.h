#pragma once

#include "ObjectManager.h"

class Application
{
    HARDLY_SCOPED_SINGLETON(Application, class ExampleHelloWorld);

    ObjectManager m_objectManager;

public:
    void init();
    void update();
};
