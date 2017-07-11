#include "dummy_gen.h"

#include "core/messages/messages.h"
#include <iostream>

using namespace std;

class dummy : public dummy_gen, public UpdatableMixin<dummy>
{
    HA_MESSAGES_IN_MIXIN(dummy)
public:
    void update(float) { cout << "UPDATE CALLED! 8" << endl; }

    static int a;
};

HA_MIXIN_DEFINE(dummy, dynamix::none);

HA_GLOBAL_MEMBER(int, dummy, a) = 5;
HA_GLOBAL(int, a) = 5;
HA_GLOBAL_STATIC(int, b);
