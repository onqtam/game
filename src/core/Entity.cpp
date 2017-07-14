#include "Entity.h"

#include "core/registry/registry.h"

HA_SINGLETON_INSTANCE(EntityManager);

void Entity::addMixin(const char* mixin) {
    auto& mixins = getMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].add(this);
}

void Entity::remMixin(const char* mixin) {
    auto& mixins = getMixins();
    hassert(mixins.find(mixin) != mixins.end());
    mixins[mixin].remove(this);
}
