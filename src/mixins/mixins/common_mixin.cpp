#include "core/registry/registry.h"

#include "mixins/messages/messages.h"
#include "common_mixin.h"

#include <iostream>

using namespace dynamix;
using namespace std;

common_mixin::common_mixin()
        : _id(0) {}

void common_mixin::set_id(int id) { _id = id; }

int common_mixin::get_id() const { return _id; }

void common_mixin::trace(std::ostream& o) const { o << " object with id " << _id << endl; }

HARDLY_MIXIN_WITHOUT_CODEGEN(common_mixin, get_id_msg& set_id_msg& priority(1000, trace_msg));
