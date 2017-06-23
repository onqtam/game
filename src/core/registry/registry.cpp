#include "precompiled.h"
// include this after "precompiled.h" because the registry target is one of the few that doesn't use the same shared pch
#include "core/registry/registry.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // _WIN32

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

static ppk::assert::implementation::AssertAction::AssertAction assertHandler(
        const char* file, int line, const char* function, const char* expression, int level,
        const char* message) {
    using namespace ppk::assert::implementation;

    char buf[2048];

    size_t num_written =
            snprintf(buf, HARDLY_COUNT_OF(buf),
                     "Assert failed!\nFile: %s\nLine: %d\nFunction: %s\nExpression: %s", file, line,
                     function, expression);

    if(message)
        snprintf(buf + num_written, HARDLY_COUNT_OF(buf) - num_written, "Message: %s\n", message);

    fprintf(stderr, "%s", buf);

    if(level < AssertLevel::Debug) {
        return static_cast<AssertAction::AssertAction>(0);
    } else if(AssertLevel::Debug <= level && level < AssertLevel::Error) {
#ifdef _WIN32

        // this might cause issues if there are multiple windows or threads
        int res = MessageBox(GetActiveWindow(), buf, "Assert failed! Break in the debugger?",
                             MB_YESNO | MB_ICONEXCLAMATION);

        if(res == 7)
            return static_cast<AssertAction::AssertAction>(0);
        return AssertAction::Break;
#else
        return AssertAction::Break;
#endif
    } else if(AssertLevel::Error <= level && level < AssertLevel::Fatal) {
        return AssertAction::Throw;
    }

    return AssertAction::Abort;
}

static int set_ppk_assert_handler = []() {
    ppk::assert::implementation::setAssertHandler(assertHandler);
    return 0;
}();
