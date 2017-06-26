#pragma once

#if defined _WIN32 || defined __CYGWIN__
#ifdef __GNUC__
#define HA_EXPORT __attribute__((dllexport))
#define HA_IMPORT __attribute__((dllimport))
#else // __GNUC__
#define HA_EXPORT __declspec(dllexport)
#define HA_IMPORT __declspec(dllimport)
#endif // __GNUC__
#else  // _WIN32
#define HA_EXPORT __attribute__((visibility("default")))
#define HA_IMPORT
#endif // _WIN32

// TODO: think about just using WINDOWS_EXPORT_ALL_SYMBOLS in cmake instead of manually annotating what to export from the executable

#ifdef HA_WITH_PLUGINS
#ifdef HA_PLUGIN
#define HAPI HA_IMPORT
#else // HA_PLUGIN
#define HAPI HA_EXPORT
#endif // HA_PLUGIN
#else  // HA_WITH_PLUGINS
#define HAPI
#endif // HA_WITH_PLUGINS
