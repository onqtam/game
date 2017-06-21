#include "InputEvent.h"

#include "core/Application.h"
#include "mixins/messages/messages.h"

void InputEventListener_add(void* in) {
    Application::get().addInputEventListener(get_id(::dynamix::object_of(in)));
}
void InputEventListener_remove(void* in) {
    Application::get().removeInputEventListener(get_id(::dynamix::object_of(in)));
}
