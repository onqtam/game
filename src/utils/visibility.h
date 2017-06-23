#pragma once

#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define SYMBOL_EXPORT __attribute__((dllexport))
#define SYMBOL_IMPORT __attribute__((dllimport))
#else // __GNUC__
#define SYMBOL_EXPORT __declspec(dllexport)
#define SYMBOL_IMPORT __declspec(dllimport)
#endif // __GNUC__
#else  // _WIN32
#define SYMBOL_EXPORT __attribute__((visibility("default")))
#define SYMBOL_IMPORT
#endif // _WIN32

// TODO: think about just using WINDOWS_EXPORT_ALL_SYMBOLS in cmake instead of manually annotating what to export from the executable

#ifdef HARDLY_WITH_PLUGINS
#ifdef HARDLY_PLUGIN
#define HAPI SYMBOL_IMPORT
#else // HARDLY_PLUGIN
#define HAPI SYMBOL_EXPORT
#endif // HARDLY_PLUGIN
#else  // HARDLY_WITH_PLUGINS
#define HAPI
#endif // HARDLY_WITH_PLUGINS
