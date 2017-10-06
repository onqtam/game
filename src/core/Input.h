#pragma once

enum class MouseButton
{
    Left,
    Right,
    Middle
};

enum class ButtonAction
{
    Release,
    Press
};

enum class KeyAction
{
    Release,
    Press,
    Repeat
};

struct KeyEvent
{
    int       type; // InputEvent::KEY
    int       key;
    KeyAction action;
    int       mods;
};

struct MouseEvent
{
    int   type; // InputEvent::Mouse
    float x;
    float y;
    float dx;
    float dy;
};

struct ButtonEvent
{
    int          type; // InputEvent::BUTTON
    MouseButton  button;
    ButtonAction action;
};

struct ScrollEvent
{
    int   type; // InputEvent::SCROLL
    float scroll;
};

union InputEvent
{
    enum
    {
        KEY,
        MOUSE,
        BUTTON,
        SCROLL
    };
    int         type;
    KeyEvent    key;
    MouseEvent  mouse;
    ButtonEvent button;
    ScrollEvent scroll;
};

struct InputEventListener;
HAPI void InputEventListener_add(InputEventListener* in);
HAPI void InputEventListener_remove(InputEventListener* in);

struct HAPI InputEventListener
{
    InputEventListener() { InputEventListener_add(this); }
    InputEventListener(const InputEventListener&) { InputEventListener_add(this); }
    virtual ~InputEventListener() { InputEventListener_remove(this); }
    InputEventListener& operator=(const InputEventListener&) = default;

    virtual void process_event(const InputEvent& ev) = 0;
};

HAPI bool isKeyDown(int key);
HAPI bool isModOn(int mod);
HAPI bool isButtonDown(MouseButton button);

// copied from GLFW

// clang-format off
#define HA_MOD_SHIFT        0x0001
#define HA_MOD_CONTROL      0x0002
#define HA_MOD_ALT          0x0004
#define HA_MOD_SUPER        0x0008

// Printable keys
#define HA_KEY_SPACE              32
#define HA_KEY_APOSTROPHE         39
#define HA_KEY_COMMA              44
#define HA_KEY_MINUS              45
#define HA_KEY_PERIOD             46
#define HA_KEY_SLASH              47
#define HA_KEY_0                  48
#define HA_KEY_1                  49
#define HA_KEY_2                  50
#define HA_KEY_3                  51
#define HA_KEY_4                  52
#define HA_KEY_5                  53
#define HA_KEY_6                  54
#define HA_KEY_7                  55
#define HA_KEY_8                  56
#define HA_KEY_9                  57
#define HA_KEY_SEMICOLON          59
#define HA_KEY_EQUAL              61
#define HA_KEY_A                  65
#define HA_KEY_B                  66
#define HA_KEY_C                  67
#define HA_KEY_D                  68
#define HA_KEY_E                  69
#define HA_KEY_F                  70
#define HA_KEY_G                  71
#define HA_KEY_H                  72
#define HA_KEY_I                  73
#define HA_KEY_J                  74
#define HA_KEY_K                  75
#define HA_KEY_L                  76
#define HA_KEY_M                  77
#define HA_KEY_N                  78
#define HA_KEY_O                  79
#define HA_KEY_P                  80
#define HA_KEY_Q                  81
#define HA_KEY_R                  82
#define HA_KEY_S                  83
#define HA_KEY_T                  84
#define HA_KEY_U                  85
#define HA_KEY_V                  86
#define HA_KEY_W                  87
#define HA_KEY_X                  88
#define HA_KEY_Y                  89
#define HA_KEY_Z                  90
#define HA_KEY_LEFT_BRACKET       91
#define HA_KEY_BACKSLASH          92
#define HA_KEY_RIGHT_BRACKET      93
#define HA_KEY_GRAVE_ACCENT       96
#define HA_KEY_WORLD_1            161
#define HA_KEY_WORLD_2            162

// Function keys
#define HA_KEY_ESCAPE             256
#define HA_KEY_ENTER              257
#define HA_KEY_TAB                258
#define HA_KEY_BACKSPACE          259
#define HA_KEY_INSERT             260
#define HA_KEY_DELETE             261
#define HA_KEY_RIGHT              262
#define HA_KEY_LEFT               263
#define HA_KEY_DOWN               264
#define HA_KEY_UP                 265
#define HA_KEY_PAGE_UP            266
#define HA_KEY_PAGE_DOWN          267
#define HA_KEY_HOME               268
#define HA_KEY_END                269
#define HA_KEY_CAPS_LOCK          280
#define HA_KEY_SCROLL_LOCK        281
#define HA_KEY_NUM_LOCK           282
#define HA_KEY_PRINT_SCREEN       283
#define HA_KEY_PAUSE              284
#define HA_KEY_F1                 290
#define HA_KEY_F2                 291
#define HA_KEY_F3                 292
#define HA_KEY_F4                 293
#define HA_KEY_F5                 294
#define HA_KEY_F6                 295
#define HA_KEY_F7                 296
#define HA_KEY_F8                 297
#define HA_KEY_F9                 298
#define HA_KEY_F10                299
#define HA_KEY_F11                300
#define HA_KEY_F12                301
#define HA_KEY_F13                302
#define HA_KEY_F14                303
#define HA_KEY_F15                304
#define HA_KEY_F16                305
#define HA_KEY_F17                306
#define HA_KEY_F18                307
#define HA_KEY_F19                308
#define HA_KEY_F20                309
#define HA_KEY_F21                310
#define HA_KEY_F22                311
#define HA_KEY_F23                312
#define HA_KEY_F24                313
#define HA_KEY_F25                314
#define HA_KEY_KP_0               320
#define HA_KEY_KP_1               321
#define HA_KEY_KP_2               322
#define HA_KEY_KP_3               323
#define HA_KEY_KP_4               324
#define HA_KEY_KP_5               325
#define HA_KEY_KP_6               326
#define HA_KEY_KP_7               327
#define HA_KEY_KP_8               328
#define HA_KEY_KP_9               329
#define HA_KEY_KP_DECIMAL         330
#define HA_KEY_KP_DIVIDE          331
#define HA_KEY_KP_MULTIPLY        332
#define HA_KEY_KP_SUBTRACT        333
#define HA_KEY_KP_ADD             334
#define HA_KEY_KP_ENTER           335
#define HA_KEY_KP_EQUAL           336
#define HA_KEY_LEFT_SHIFT         340
#define HA_KEY_LEFT_CONTROL       341
#define HA_KEY_LEFT_ALT           342
#define HA_KEY_LEFT_SUPER         343
#define HA_KEY_RIGHT_SHIFT        344
#define HA_KEY_RIGHT_CONTROL      345
#define HA_KEY_RIGHT_ALT          346
#define HA_KEY_RIGHT_SUPER        347
#define HA_KEY_MENU               348
// clang-format on
