#pragma once

#include "InputEvent.h"

struct GLFWwindow;

class Application
{
    HARDLY_SCOPED_SINGLETON(Application, int main(int, char**));

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow*, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow*, int key, int, int action, int mods);
    static void charCallback(GLFWwindow*, unsigned int c);
    static void cursorPosCallback(GLFWwindow*, double x, double y);

    friend void imguiEvents(float dt);

    void addInputEvent(const InputEvent& ev) { m_inputs.push_back(ev); }

public:
    uint32 getWidth() const { return mWidth; }
    uint32 getHeight() const { return mHeight; }
    float    getDt() const { return dt; }

    void addInputEventListener(int in);
    void removeInputEventListener(int in);

    int run(int argc, char** argv);
    void processEvents();
    void update();
    void reset(uint32 flags = 0);

private:
    std::vector<InputEvent> m_inputs;
    std::vector<int>        m_inputEventListeners;

    GLFWwindow* mWindow;
    uint32    mReset;
    uint32    mWidth           = 1280;
    uint32    mHeight          = 768;
    bool        mMousePressed[3] = {false, false, false};
    float       mMouseWheel      = 0.0f;

    float lastTime = 0.f;
    float dt       = 0.f;
    float time     = 0.f;
};
