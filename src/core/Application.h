#pragma once

#include "ObjectManager.h"

class Application
{
    HARDLY_SCOPED_SINGLETON(Application, int main(int, char**));

    ObjectManager m_objectManager;

public:
    void init();
    void update();
};
