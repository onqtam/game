#include "core/registry/registry.h"

#include "mixins/messages/messages.h"

#include "camera_gen.h"

#include <iostream>

using namespace dynamix;

class camera : public camera_gen
{
    HARDLY_MESSAGES_IN_MIXIN(common)
public:
};

HARDLY_MIXIN(camera, dynamix::none);
