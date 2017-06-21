#include "common_gen.h"

#include "core/registry/registry.h"

#include "mixins/messages/messages.h"

#include <iostream>

using namespace dynamix;
//using namespace std;

class common : public common_gen
{
    HARDLY_MESSAGES_IN_MIXIN(common)
public:
    void trace(std::ostream& o) const;

    void set_id(int in) { id = in; }
    int             get_id() const { return id; }
    void set_pos(const glm::vec3& in) { pos = in; }
    const glm::vec3&              get_pos() const { return pos; }
};

void common::trace(std::ostream& o) const { o << " object with id " << id << std::endl; }

HARDLY_MIXIN(common, get_id_msg& set_id_msg& get_pos_msg& set_pos_msg& priority(1000, trace_msg));
