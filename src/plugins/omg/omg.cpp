#include "core/messages/messages.h"
#include <iostream>

#include "omg_gen.h"

using namespace std;

class omg : public omg_gen
{
    HA_MESSAGES_IN_MIXIN(omg)
public:
    void trace(ostream& o) const { o << "\twith a OMG plugin mixin" << endl; }
};

HA_MIXIN_DEFINE(omg, trace_msg);
