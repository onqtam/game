#include "core/registry/registry.h"

HAPI MixinInfoMap& getMixins() {
    static MixinInfoMap data;
    return data;
}

HAPI int registerMixin(const char* name, MixinInfo info) {
    getMixins()[name] = info;
    return 0;
}

HAPI GlobalInfoMap& getGlobals() {
    static GlobalInfoMap data;
    return data;
}

HAPI int registerGlobal(const char* name, GlobalInfo info) {
    getGlobals()[name] = info;
    return 0;
}

static int set_ppk_assert_handler = Utils::setPPKAssertHandler();
