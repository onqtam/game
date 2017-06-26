#pragma once

// clang-format off

#if defined(__clang__)
#define HA_SUPPRESS_WARNINGS_END _Pragma("clang diagnostic pop")
#define HA_SUPPRESS_WARNINGS \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wgnu-anonymous-struct\"") \
    _Pragma("clang diagnostic ignored \"-Wnested-anon-types\"") \
    _Pragma("clang diagnostic ignored \"-Wreturn-type-c-linkage\"") \
    _Pragma("clang diagnostic ignored \"-Wshadow\"") \
    _Pragma("clang diagnostic ignored \"-Wdocumentation-unknown-command\"") \
    _Pragma("clang diagnostic ignored \"-Wreserved-id-macro\"") \
    _Pragma("clang diagnostic ignored \"-Wundef\"") \
    _Pragma("clang diagnostic ignored \"-Wdeprecated\"") \
    _Pragma("clang diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("clang diagnostic ignored \"-Wexit-time-destructors\"") \
    _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"") \
    _Pragma("clang diagnostic ignored \"-Wnewline-eof\"") \
    _Pragma("clang diagnostic ignored \"-Wdocumentation\"") \
    _Pragma("clang diagnostic ignored \"-Wweak-vtables\"") \
    _Pragma("clang diagnostic ignored \"-Wundefined-reinterpret-cast\"") \
    _Pragma("clang diagnostic ignored \"-Wconversion\"") \
    _Pragma("clang diagnostic ignored \"-Wcovered-switch-default\"") \
    _Pragma("clang diagnostic ignored \"-Wformat-nonliteral\"") \
    _Pragma("clang diagnostic ignored \"-Wignored-qualifiers\"") \
    _Pragma("clang diagnostic ignored \"-Wunused-parameter\"") \

#endif // __clang__

#if defined(__GNUC__) && !defined(__clang__)
#define HA_SUPPRESS_WARNINGS_END _Pragma("GCC diagnostic pop")
#define HA_SUPPRESS_WARNINGS \
    _Pragma("GCC diagnostic push") \
    _Pragma("GCC diagnostic ignored \"-Wconversion\"") \
    _Pragma("GCC diagnostic ignored \"-Weffc++\"") \
    _Pragma("GCC diagnostic ignored \"-Wshadow\"") \
    _Pragma("GCC diagnostic ignored \"-Wsign-conversion\"") \
    _Pragma("GCC diagnostic ignored \"-Wstrict-overflow\"") \
    _Pragma("GCC diagnostic ignored \"-Wmissing-declarations\"") \
    _Pragma("GCC diagnostic ignored \"-Winline\"") \
    _Pragma("GCC diagnostic ignored \"-Wswitch-default\"") \
    _Pragma("GCC diagnostic ignored \"-Wunsafe-loop-optimizations\"") \
    _Pragma("GCC diagnostic ignored \"-Wzero-as-null-pointer-constant\"") \
    _Pragma("GCC diagnostic ignored \"-Wctor-dtor-privacy\"") \
    _Pragma("GCC diagnostic ignored \"-Wpedantic\"") \
    _Pragma("GCC diagnostic ignored \"-Wuseless-cast\"") \
    _Pragma("GCC diagnostic ignored \"-Woverflow\"") \
    _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("GCC diagnostic ignored \"-Wignored-qualifiers\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("GCC diagnostic ignored \"-Wnon-virtual-dtor\"") \


#endif // __GNUC__

#ifdef _MSC_VER
#define HA_SUPPRESS_WARNINGS_END __pragma(warning(pop))
#define HA_SUPPRESS_WARNINGS __pragma(warning(push, 0))
#endif // _MSC_VER

// clang-format on
