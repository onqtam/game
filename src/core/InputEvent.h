#pragma once

struct KeyEvent
{
    int type; // InputEvent::KEY
    int key;
    int action; // GLFW_PRESS (1) / GLFW_RELEASE (0)
    int mods;
};

struct MotionEvent
{
    int    type; // InputEvent::MOTION
    double x;
    double y;
    double dx;
    double dy;
};

struct ButtonEvent
{
    int type;   // InputEvent::BUTTON
    int button; // 0 / 1 / 2
    int action; // GLFW_PRESS (1) / GLFW_RELEASE (0)
};

struct ScrollEvent
{
    int    type;   // InputEvent::SCROLL
    double scroll;
};

union InputEvent
{
    enum
    {
        KEY,
        MOTION,
        BUTTON,
        SCROLL
    };
    int         type;
    KeyEvent    key;
    MotionEvent motion;
    ButtonEvent button;
    ScrollEvent scroll;
};

template <typename T>
struct InputEventListener
{
    InputEventListener() { InputEventListener_add(static_cast<T*>(this)); }
    ~InputEventListener() { InputEventListener_remove(static_cast<T*>(this)); }
    friend void InputEventListener_add(void* in);
    friend void InputEventListener_remove(void* in);
};
