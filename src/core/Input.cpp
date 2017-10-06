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
        case HA_MOD_SHIFT: return io.KeyShift; break;
        case HA_MOD_CONTROL: return io.KeyCtrl; break;
        case HA_MOD_ALT: return io.KeyAlt; break;
        case HA_MOD_SUPER: return io.KeySuper; break;
        default: break;
    }
    hassert(false);
    return false;
}

bool isButtonDown(MouseButton button) {
    ImGuiIO& io = ImGui::GetIO();
    switch(button) {
        case MouseButton::Left: return io.MouseDown[0]; break;
        case MouseButton::Right: return io.MouseDown[1]; break;
        case MouseButton::Middle: return io.MouseDown[2]; break;
        default: break;
    }
    hassert(false);
    return false;
}
