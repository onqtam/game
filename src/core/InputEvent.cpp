#include "InputEvent.h"

#include "core/Application.h"

void InputEventListener_add(InputEventListener* in) {
    Application::get().addInputEventListener(in);
}
void InputEventListener_remove(InputEventListener* in) {
    Application::get().removeInputEventListener(in);
}
