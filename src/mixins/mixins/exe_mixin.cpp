#include "core/registry/registry.h"

#include "mixins/messages/messages.h"
#include "exe_mixin.h"

#include <iostream>

using namespace std;

void exe_mixin::trace(std::ostream& o) const { o << "\twith exe_mixin" << endl; }

HARDLY_MIXIN_WITHOUT_CODEGEN(exe_mixin, trace_msg);
