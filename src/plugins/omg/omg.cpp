#include "core/messages/messages.h"

#include "omg_gen.h"

using namespace std;

class omg : public omg_gen
{
    HA_MESSAGES_IN_MIXIN(omg)
public:
};

HA_MIXIN_DEFINE(omg, dynamix::none);
