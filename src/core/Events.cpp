#include "Input.h"

#include "core/Application.h"

void FrameBeginListener_add(FrameBeginEventListener* in) {
    Application::get().addFrameBeginEventListener(in);
}
void FrameBeginListener_remove(FrameBeginEventListener* in) {
    Application::get().removeFrameBeginEventListener(in);
}
