#include "common_gen.h"

#include "core/registry/registry.h"

#include "core/messages/messages.h"

#include <iostream>

using namespace dynamix;
//using namespace std;

class common : public common_gen
{
    HARDLY_MESSAGES_IN_MIXIN(common)
public:
    void trace(std::ostream& o) const;

    void set_pos(const glm::vec3& in) { pos = in; }
    const glm::vec3&              get_pos() const { return pos; }
};

void common::trace(std::ostream& o) const { o << " object with id " << ha_this.id() << std::endl; }

HARDLY_MIXIN(common, get_pos_msg& set_pos_msg& priority(1000, trace_msg));
