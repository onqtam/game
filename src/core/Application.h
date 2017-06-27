#pragma once

#include "InputEvent.h"

struct GLFWwindow;

class HAPI Application
{
    HA_SCOPED_SINGLETON(Application, int main(int, char**));

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow*, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow*, int key, int, int action, int mods);
    static void charCallback(GLFWwindow*, unsigned int c);
    static void cursorPosCallback(GLFWwindow*, double x, double y);

    friend void imguiEvents(float dt);

    void addInputEvent(const InputEvent& ev) { m_inputs.push_back(ev); }

    int run(int argc, char** argv);
    void processEvents();
    void update();
    void reset(uint32 flags = 0);

public:
    uint32 width() const { return m_width; }
    uint32 height() const { return m_height; }
    float  dt() const { return m_dt; }

    void addInputEventListener(eid in);
    void removeInputEventListener(eid in);

private:
    std::vector<InputEvent> m_inputs;
    std::vector<eid>        m_inputEventListeners;

    GLFWwindow* m_window;
    uint32      m_reset;
    uint32      m_width           = 1280;
    uint32      m_height          = 768;
    bool        m_mousePressed[3] = {false, false, false};
    float       m_mouseWheel      = 0.0f;

    float m_lastTime = 0.f;
    float m_dt       = 0.f;
    float m_time     = 0.f;
};
