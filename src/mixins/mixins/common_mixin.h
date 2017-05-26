#pragma once

#include "common_mixin_fwd.h"

class common_mixin
{
public:
    common_mixin();

    void trace(std::ostream& o) const;

    void set_id(int id);
    int get_id() const;

private:
    int _id;
};
