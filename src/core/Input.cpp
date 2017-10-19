#include "Input.h"

#include "core/Application.h"

void InputEventListener_add(InputEventListener* in) {
    Application::get().addInputEventListener(in);
}
void InputEventListener_remove(InputEventListener* in) {
    Application::get().removeInputEventListener(in);
}

bool isKeyDown(int key) {
    ImGuiIO& io = ImGui::GetIO();
    return io.KeysDown[key];
}

bool isModOn(int mod) {
    ImGuiIO& io = ImGui::GetIO();
    switch(mod) {
        case HA_MOD_SHIFT: return io.KeyShift;
        case HA_MOD_CONTROL: return io.KeyCtrl;
        case HA_MOD_ALT: return io.KeyAlt;
        case HA_MOD_SUPER: return io.KeySuper;
        default: break;
    }
    hassert(false);
    return false;
}

bool isButtonDown(MouseButton button) {
    ImGuiIO& io = ImGui::GetIO();
    HA_CLANG_SUPPRESS_WARNING("-Wcovered-switch-default")
    switch(button) {
        case MouseButton::Left: return io.MouseDown[0];
        case MouseButton::Right: return io.MouseDown[1];
        case MouseButton::Middle: return io.MouseDown[2];
        default: break;
    }
    HA_CLANG_SUPPRESS_WARNING_END
    hassert(false);
    return false;
}
