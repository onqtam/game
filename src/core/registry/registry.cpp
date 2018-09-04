#include "precompiled.h"

extern "C" HA_SYMBOL_EXPORT MixinInfoMap& getMixins() {
    static MixinInfoMap data;
    return data;
}

MixinInfoMap& getMixins_Local() { return getMixins(); }

int registerMixin(cstr name, MixinInfo info) {
    getMixins()[name] = info;
    return 0;
}

extern "C" HA_SYMBOL_EXPORT GlobalInfoMap& getGlobals() {
    static GlobalInfoMap data;
    return data;
}

int registerGlobal(cstr name, GlobalInfo info) {
    getGlobals()[name] = info;
    return 0;
}
