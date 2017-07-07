#pragma once

#include "core/GraphicsHelpers.h"

class HAPI ObjectManager
{
    HA_SCOPED_SINGLETON(ObjectManager);
    ObjectManager() = default;
    friend class Application;

private:
    void init();
    void update();
    int  shutdown();

    DebugMeshHandle asd;
    ShaderHandle mProgram;
    ShaderHandle mProgram2;

public:
    eid m_camera;
};
