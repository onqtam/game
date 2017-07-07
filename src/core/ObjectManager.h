#pragma once

class HAPI ObjectManager
{
    HA_SCOPED_SINGLETON(ObjectManager);
    ObjectManager() = default;
    friend class Application;

    // TODO: not like this!
    friend class Editor;

private:
    void init();
    void update();
    int  shutdown();

public:
    eid m_camera;
};
