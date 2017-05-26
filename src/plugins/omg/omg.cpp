#include "mixins/messages/messages.h"
#include <iostream>

#include "omg_gen.h"

using namespace std;

class omg : public omg_gen
{
    HARDLY_MESSAGES_IN_MIXIN(omg)
public:
    void trace(ostream& o) const { o << "\twith a OMG plugin mixin" << endl; }
};

HARDLY_MIXIN(omg, trace_msg);
