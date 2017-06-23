#include "InputEvent.h"

#include "core/Application.h"
#include "core/messages/messages.h"

HAPI void InputEventListener_add(void* in) {
    Application::get().addInputEventListener(Entity::cast_to_entity(in).id());
}
HAPI void InputEventListener_remove(void* in) {
    Application::get().removeInputEventListener(Entity::cast_to_entity(in).id());
}
