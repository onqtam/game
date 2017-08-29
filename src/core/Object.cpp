#include "core/registry/registry.h"
#include "core/serialization/serialization.h"
#include "core/imgui/imgui_stuff.h"

HA_SINGLETON_INSTANCE(ObjectManager);

void Object::addMixin(const char* mixin) {
    auto& mixins = getMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].add(this);
}

void Object::remMixin(const char* mixin) {
    auto& mixins = getMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].remove(this);
}

// included here and not in Object.h to contain dependencies
#include <gen/Object.h.inl>
