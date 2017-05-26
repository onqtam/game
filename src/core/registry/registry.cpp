#include "core/registry/registry.h"

extern "C" SYMBOL_EXPORT MixinInfoMap& getMixins() {
    static MixinInfoMap data;
    return data;
}

int registerMixin(const char* name, MixinInfo info) {
    getMixins()[name] = info;
    return 0;
}

extern "C" SYMBOL_EXPORT GlobalInfoMap& getGlobals() {
    static GlobalInfoMap data;
    return data;
}

int registerGlobal(const char* name, GlobalInfo info) {
    getGlobals()[name] = info;
    return 0;
}

static int set_ppk_assert_handler = Utils::setPPKAssertHandler();
