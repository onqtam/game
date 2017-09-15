#include "core/serialization/serialization_common.h"
#include "core/imgui/imgui_bindings_common.h"

HA_SINGLETON_INSTANCE(ObjectManager);

void Object::addMixin(cstr mixin) {
    auto& mixins = getMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].add(this);
}

void Object::remMixin(cstr mixin) {
    auto& mixins = getMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].remove(this);
}

// included here and not in Object.h to contain dependencies
#include <gen/Object.h.inl>
