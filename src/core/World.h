#pragma once

class HAPI World : public Singleton<World>
{
    HA_SINGLETON(World);

private:
    float m_width  = 100.f;
    float m_height = 100.f;

    oid m_camera;
    oid m_editor;

public:
    World();

    void update();

    oid     camera() { return m_camera; }
    Object& editor() { return m_editor.obj(); }

    float width() const { return m_width; }
    float height() const { return m_height; }
};
