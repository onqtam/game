#include "core/registry/registry.h"
#include "core/InputEvent.h"

#include "mixins/messages/messages.h"

#include "camera_gen.h"

#include <iostream>
#include <utility>

using namespace dynamix;

class camera :  public camera_gen, public InputEventListener<camera>
{
    HARDLY_MESSAGES_IN_MIXIN(common)
public:

    void process_event(const InputEvent& ev) {
        std::cout << "event!\n";
    }
};

HARDLY_MIXIN(camera, process_event_msg);
