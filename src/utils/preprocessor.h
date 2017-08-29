#pragma once

#define HA_EMPTY()
#define HA_USE(x)
#define HA_EXPAND(x) x
#define HA_TOSTR_IMPL(x) #x
#define HA_TOSTR(x) HA_TOSTR_IMPL(x)

// clang-format off
#define HA_CAT_IMPL(s1, s2) s1##s2
#define HA_CAT_1(s1, s2) HA_CAT_IMPL(s1, s2)
#define HA_CAT_2(s1, s2) HA_EXPAND(s1)HA_EXPAND(s2) // important to not have a space between the 2 expands
// clang-format on

#define HA_ANONYMOUS(x) HA_CAT_1(x, __COUNTER__)

#ifdef _MSC_VER
#define HA_PRAGMA(x) __pragma(x)
#define HA_MESSAGE_IMPL(x) HA_PRAGMA(message(__FILE__ "(" HA_TOSTR(__LINE__) ")" x))
#else // _MSC_VER
#define HA_PRAGMA(x) _Pragma(#x)
#define HA_MESSAGE_IMPL(x)
#endif // _MSC_VER
#define HA_MESSAGE(x) HA_MESSAGE_IMPL(": " x)
#define HA_WARNING(x) HA_MESSAGE_IMPL(": Warning: " x)
#define HA_ERROR(x) HA_MESSAGE_IMPL(": Error: " x)
#define HA_TODO(x) HA_MESSAGE_IMPL(": TODO: " x)

#define TODO HA_TODO

// == THIS IS UNNECESSARY SINCE THESE WARNINGS ARE DISABLED (atleast the one for clang)
//#if defined(__clang__)
//#define HA_UNUSED_GLOBAL_NO_WARNINGS(var) _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"") static int var
//#define HA_UNUSED_GLOBAL_NO_WARNINGS_END() _Pragma("clang diagnostic pop")
//#elif defined(__GNUC__)
//#define HA_UNUSED_GLOBAL_NO_WARNINGS(var) static int var __attribute__((unused))
//#define HA_UNUSED_GLOBAL_NO_WARNINGS_END()
//#else // MSVC / other
//#define HA_UNUSED_GLOBAL_NO_WARNINGS(var) static int var
//#define HA_UNUSED_GLOBAL_NO_WARNINGS_END()
//#endif // MSVC / other

#ifdef _MSC_VER
#define HA_COUNT_OF(x) _countof(x)
#else // _MSC_VER
// http://blogs.msdn.com/b/the1/archive/2004/05/07/128242.aspx
template <typename T, size_t N>
char (&_ArraySizeHelper(T (&array)[N]))[N];
#define HA_COUNT_OF(array) (sizeof(_ArraySizeHelper(array)))
#endif // _MSC_VER

// https://blogs.msdn.microsoft.com/vcblog/2016/03/30/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/
#ifdef _MSC_VER
#define HA_EMPTY_BASE __declspec(empty_bases)
#else // _MSC_VER
#define HA_EMPTY_BASE
#endif // _MSC_VER

template<int s> struct print_ct;
#define HA_PRINT_CT(x) print_ct<x> HA_ANONYMOUS(ct_)

#ifdef _MSC_VER
#define HA_NOINLINE __declspec(noinline)
#else // _MSC_VER
#define HA_NOINLINE __attribute__((noinline))
#endif // _MSC_VER

// helpers that don't expand to anything - used by the type parser
#define FIELD           // indicates the start of a field definition inside of a type
#define EXPORT          // attribute - indicates that the field should be exported
#define NO_SKIP         // attribute - emit reflection for the class even if without any fields
#define ATTRIBUTES(...) // list attributes and tags in a comma-separated fashion using this
