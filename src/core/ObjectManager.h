#pragma once

#include "core/GraphicsHelpers.h"

class HAPI ObjectManager
{
    HA_SINGLETON(ObjectManager);
    ObjectManager() = default;
    friend class Application;

private:
    void init();
    void update();
    int  shutdown();

    Entity editor;

    GeomHandle asd;
    ShaderHandle mProgram;

public:
    eid m_camera;
};
