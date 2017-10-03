#include "core/serialization/serialization_common.h"
#include "core/imgui/imgui_bindings_common.h"

HA_SINGLETON_INSTANCE(ObjectManager);

void Object::addMixin(cstr mixin) {
    auto& mixins = getAllMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].add(this);
}

void Object::remMixin(cstr mixin) {
    auto& mixins = getAllMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].remove(this);
}

HAPI void Object::set_transform(const transform& in) {
    auto parent = get_parent(*this);
    if(parent) {
        auto child_local = in.multiply(parent.obj().get_transform().inverse());
        set_transform_local(child_local);
    } else {
        set_transform_local(in);
    }
}
HAPI transform Object::get_transform() const {
    transform my = get_transform_local();
    auto      parent = get_parent(*this);
    if(parent)
        return my.multiply(parent.obj().get_transform());
    else
        return my;
}

// included here and not in Object.h to contain dependencies
#include <gen/Object.h.inl>
