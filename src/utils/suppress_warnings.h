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
    _Pragma("clang diagnostic ignored \"-Wcast-align\"") \
    _Pragma("clang diagnostic ignored \"-Wgnu-zero-variadic-macro-arguments\"") \
    _Pragma("clang diagnostic ignored \"-Wnon-virtual-dtor\"") \
    _Pragma("clang diagnostic ignored \"-Wreorder\"") \
    _Pragma("clang diagnostic ignored \"-Wfloat-equal\"") \
    _Pragma("clang diagnostic ignored \"-Wundefined-func-template\"") \

#define HA_CLANG_SUPPRESS_WARNING(w) _Pragma("clang diagnostic push") HA_PRAGMA(clang diagnostic ignored w)
#define HA_CLANG_SUPPRESS_WARNING_END _Pragma("clang diagnostic pop")
#define HA_GCC_SUPPRESS_WARNING(w)
#define HA_GCC_SUPPRESS_WARNING_END
#define HA_MSVC_SUPPRESS_WARNING(w)
#define HA_MSVC_SUPPRESS_WARNING_END

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
    _Pragma("GCC diagnostic ignored \"-Wfloat-equal\"") \
    _Pragma("GCC diagnostic ignored \"-Woverflow\"") \
    _Pragma("GCC diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("GCC diagnostic ignored \"-Wignored-qualifiers\"") \
    _Pragma("GCC diagnostic ignored \"-Wunused-parameter\"") \
    _Pragma("GCC diagnostic ignored \"-Wnon-virtual-dtor\"") \
    _Pragma("GCC diagnostic ignored \"-Wstrict-aliasing\"") \
    _Pragma("GCC diagnostic ignored \"-Wreorder\"") \
    _Pragma("GCC diagnostic ignored \"-Wnull-dereference\"") \

#define HA_CLANG_SUPPRESS_WARNING(w)
#define HA_CLANG_SUPPRESS_WARNING_END
#define HA_GCC_SUPPRESS_WARNING(w) _Pragma("GCC diagnostic push") HA_PRAGMA(GCC diagnostic ignored w)
#define HA_GCC_SUPPRESS_WARNING_END _Pragma("GCC diagnostic pop")
#define HA_MSVC_SUPPRESS_WARNING(w)
#define HA_MSVC_SUPPRESS_WARNING_END

#endif // __GNUC__

#ifdef _MSC_VER
#define HA_SUPPRESS_WARNINGS_END __pragma(warning(pop))
#define HA_SUPPRESS_WARNINGS                                                   \
    __pragma(warning(push, 0))      /* all */                                  \
    __pragma(warning(disable:4715)) /* not all control paths return a value */ \
    __pragma(warning(disable:4702)) /* unreachable code */                     \

#define HA_CLANG_SUPPRESS_WARNING(w)
#define HA_CLANG_SUPPRESS_WARNING_END
#define HA_GCC_SUPPRESS_WARNING(w)
#define HA_GCC_SUPPRESS_WARNING_END
#define HA_MSVC_SUPPRESS_WARNING(w) __pragma(warning(push)) __pragma(warning(disable: w))
#define HA_MSVC_SUPPRESS_WARNING_END __pragma(warning(pop))

#endif // _MSC_VER

// clang-format on
