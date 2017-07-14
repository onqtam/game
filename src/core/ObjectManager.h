#pragma once

#include "core/GraphicsHelpers.h"

class HAPI ObjectManager : public Singleton<ObjectManager>
{
    HA_SINGLETON(ObjectManager);
    ObjectManager()
            : Singleton(this) {}
    friend class Application;

private:
    void init();
    void update();
    int  shutdown();

    Entity editor;

    GeomHandle   cube;
    ShaderHandle mProgram;

public:
    eid m_camera;
};
