#pragma once

#ifdef _WIN32
#define HA_SYMBOL_EXPORT __declspec(dllexport)
#define HA_SYMBOL_IMPORT __declspec(dllimport)
#else // _WIN32
#define HA_SYMBOL_EXPORT __attribute__((visibility("default")))
#define HA_SYMBOL_IMPORT
#endif // _WIN32

// TODO: think about just using WINDOWS_EXPORT_ALL_SYMBOLS in cmake instead of manually annotating what to export from the executable

#ifndef HAPI
#ifdef HA_WITH_PLUGINS
#define HAPI HA_SYMBOL_IMPORT
#else // HA_WITH_PLUGINS
#define HAPI
#endif // HA_WITH_PLUGINS
#endif // HAPI
