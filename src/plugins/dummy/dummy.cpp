#include "core/messages/messages.h"
#include <iostream>

#include "dummy_gen.h"

using namespace std;

class dummy : public dummy_gen, public UpdatableMixin<dummy>
{
    HA_MESSAGES_IN_MIXIN(dummy)
public:
    void trace(ostream& o) const { o << "\twith a :(        35       plugin mixin" << endl; }

    void update(float) { cout << "UPDATE CALLED! 8" << endl; }

    static int a;
};

HA_MIXIN_DEFINE(dummy, trace_msg);

HA_GLOBAL_MEMBER(int, dummy, a) = 5;
HA_GLOBAL(int, a) = 5;
HA_GLOBAL_STATIC(int, b);
