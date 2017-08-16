#include "Object.h"

#include "core/registry/registry.h"

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
