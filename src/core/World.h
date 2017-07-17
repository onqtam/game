#pragma once

class HAPI World : public Singleton<World>
{
    HA_SINGLETON(World);

private:
    float m_width  = 20.f;
    float m_height = 20.f;

    eid    m_camera;
    Entity m_editor;

public:
    World();

    void update();
    
    eid camera() const { return m_camera; }

    float width() const { return m_width; }
    float height() const { return m_height; }
};
