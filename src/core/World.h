#pragma once

class HAPI World
{
    HA_SCOPED_SINGLETON(World);

private:
    
    float m_width = 20.f;
    float m_height = 20.f;

public:
    
    float width() const { return m_width; }
    float height() const { return m_height; }
};
