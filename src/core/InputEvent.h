#pragma once

struct MouseButtonEvent
{
    int type;   // InputEvent::MOUSE_BUTTON
    int button; // 0 / 1 / 2
    int action; // GLFW_PRESS (1) / GLFW_RELEASE (0)
};

struct KeyboardEvent
{
    int type; // InputEvent::KEYPRESS
    int key;
    int action; // GLFW_PRESS (1) / GLFW_RELEASE (0)
    int mods;
};

struct MouseMotionEvent
{
    int    type; // InputEvent::MOUSE_MOTION
    double x;
    double y;
    double dx;
    double dy;
    double scroll;
};

union InputEvent
{
    enum
    {
        KEYPRESS,
        MOUSE_MOTION,
        MOUSE_BUTTON
    };
    int              type;
    KeyboardEvent    key;
    MouseMotionEvent motion;
    MouseButtonEvent button;
};
