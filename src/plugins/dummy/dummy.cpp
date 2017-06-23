#include "core/messages/messages.h"
#include <iostream>

#include "dummy_gen.h"

using namespace std;

class dummy : public dummy_gen, public UpdatableMixin<dummy>
{
    HARDLY_MESSAGES_IN_MIXIN(dummy)
public:
    void trace(ostream& o) const { o << "\twith a :(        35       plugin mixin" << endl; }

    void update() { cout << "UPDATE CALLED! 8" << endl; }

    static int a;
};

HARDLY_MIXIN(dummy, trace_msg);

HARDLY_GLOBAL_MEMBER(int, dummy, a) = 5;
HARDLY_GLOBAL(int, a) = 5;
HARDLY_GLOBAL_STATIC(int, b);
