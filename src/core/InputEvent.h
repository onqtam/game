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

struct InputEventListener;
HAPI void InputEventListener_add(InputEventListener* in);
HAPI void InputEventListener_remove(InputEventListener* in);

struct HAPI InputEventListener
{
    InputEventListener() { InputEventListener_add(this); }
    ~InputEventListener() { InputEventListener_remove(this); }
    InputEventListener(const InputEventListener&) = default;
    InputEventListener& operator=(const InputEventListener&) = default;

    virtual void process_event(const InputEvent& ev) = 0;
};
