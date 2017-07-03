#include "core/messages/messages.h"

#include "trololo_gen.h"

using namespace std;

class trololo : public trololo_gen
{
    HA_MESSAGES_IN_MIXIN(trololo)
public:
};

HA_MIXIN_DEFINE(trololo, dynamix::none);
