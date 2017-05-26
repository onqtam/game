#pragma once

#define HARDLY_EMPTY()
#define HARDLY_EXPAND(x) x
#define HARDLY_TOSTR_IMPL(x) #x
#define HARDLY_TOSTR(x) HARDLY_TOSTR_IMPL(x)

// clang-format off
#define HARDLY_CAT_IMPL(s1, s2) s1##s2
#define HARDLY_CAT_1(s1, s2) HARDLY_CAT_IMPL(s1, s2)
#define HARDLY_CAT_2(s1, s2) HARDLY_EXPAND(s1)HARDLY_EXPAND(s2) // important to not have a space between the 2 expands
// clang-format on

#define HARDLY_ANONYMOUS(x) HARDLY_CAT_1(x, __COUNTER__)

#ifdef _MSC_VER
#define HARDLY_PRAGMA(x) __pragma(x)
#define HARDLY_MESSAGE_IMPL(x) HARDLY_PRAGMA(message(__FILE__ "(" HARDLY_TOSTR(__LINE__) ")" x))
#else // _MSC_VER
#define HARDLY_PRAGMA(x) _Pragma(#x)
#define HARDLY_MESSAGE_IMPL(x)
#endif // _MSC_VER
#define HARDLY_MESSAGE(x) HARDLY_MESSAGE_IMPL(": " x)
#define HARDLY_WARNING(x) HARDLY_MESSAGE_IMPL(": Warning: " x)
#define HARDLY_ERROR(x) HARDLY_MESSAGE_IMPL(": Error: " x)
#define HARDLY_TODO(x) HARDLY_MESSAGE_IMPL(": TODO: " x)

#define TODO HARDLY_TODO

// == THIS IS UNNECESSARY SINCE THESE WARNINGS ARE DISABLED (atleast the one for clang)
//#if defined(__clang__)
//#define HARDLY_UNUSED_GLOBAL_NO_WARNINGS(var) _Pragma("clang diagnostic push") _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"") static int var
//#define HARDLY_UNUSED_GLOBAL_NO_WARNINGS_END() _Pragma("clang diagnostic pop")
//#elif defined(__GNUC__)
//#define HARDLY_UNUSED_GLOBAL_NO_WARNINGS(var) static int var __attribute__((unused))
//#define HARDLY_UNUSED_GLOBAL_NO_WARNINGS_END()
//#else // MSVC / other
//#define HARDLY_UNUSED_GLOBAL_NO_WARNINGS(var) static int var
//#define HARDLY_UNUSED_GLOBAL_NO_WARNINGS_END()
//#endif // MSVC / other

#ifdef _MSC_VER
#define HARDLY_COUNT_OF(x) _countof(x)
#else // _MSC_VER
// http://blogs.msdn.com/b/the1/archive/2004/05/07/128242.aspx
template <typename T, size_t N>
char (&_ArraySizeHelper(T (&array)[N]))[N];
#define HARDLY_COUNT_OF(array) (sizeof(_ArraySizeHelper(array)))
#endif // _MSC_VER
