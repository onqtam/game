#pragma once

#include <GLFW/glfw3.h>

#include "core/ObjectManager.h"

class Application
{
    HARDLY_SCOPED_SINGLETON(Application, int main(int, char**));

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow*, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow*, int key, int, int action, int mods);
    static void charCallback(GLFWwindow*, unsigned int c);
    static void cursorPosCallback(GLFWwindow*, double x, double y);

    void imguiEvents(float dt);

public:
    int run(int argc, char** argv);

    void reset(uint32_t flags = 0);
    uint32_t getWidth() const { return mWidth; }
    uint32_t getHeight() const { return mHeight; }

    void initialize(int argc, char** argv);
    void update();
    int  shutdown() const;
    void onReset() const;

private:
    GLFWwindow* mWindow;
    uint32_t    mReset;
    uint32_t    mWidth           = 1280;
    uint32_t    mHeight          = 768;
    bool        mMousePressed[3] = {false, false, false};
    float       mMouseWheel      = 0.0f;

    float lastTime = 0.f;
    float dt       = 0.f;
    float time     = 0.f;

    ObjectManager m_objectManager;
};
