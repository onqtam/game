#include "mixins/messages/messages.h"
#include <iostream>

#include "trololo_gen.h"

using namespace std;

class trololo : public trololo_gen
{
    HARDLY_MESSAGES_IN_MIXIN(trololo)
public:
    void trace(ostream& o) const { o << "\twith a TROLOLO_LOL plugin mixin" << endl; }
};

HARDLY_MIXIN(trololo, trace_msg);
