#include "mixins/messages/messages.h"
#include <iostream>

#include "deymn_gen.h"

using namespace std;

class deymn : public deymn_gen
{
    HARDLY_MESSAGES_IN_MIXIN(deymn)
public:
    void trace(ostream& o) const { o << "\twith a :(        35       plugin mixin" << endl; }

};

HARDLY_MIXIN(deymn, trace_msg);
