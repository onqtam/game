#include "precompiled.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _WIN32

extern "C" HA_SYMBOL_EXPORT MixinInfoMap& getMixins() {
    static MixinInfoMap data;
    return data;
}

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
