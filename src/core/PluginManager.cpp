#ifdef HA_WITH_PLUGINS

#include "PluginManager.h"

#include "ObjectManager.h"
#include "core/registry/registry.h"
#include "utils/utils.h"
#include "serialization/serialization.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN

HA_SUPPRESS_WARNINGS

#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>

HA_SUPPRESS_WARNINGS_END

typedef HMODULE    DynamicLib;
static const char* orig_plugin_tail   = "_plugin.dll";
static const char* copied_plugin_tail = "_copied.dll";
static const int   plugin_ext_len     = 4;

#define LoadDynlib(lib) LoadLibrary(lib) // ".dll")
#define CloseDynlib FreeLibrary
#define GetProc GetProcAddress
#define CopyDynlib(src, dst) CopyFile(src, dst, false)

#define cast_to_dynlib(x) static_cast<DynamicLib>(x)

#else // _WIN32

#include <dlfcn.h>
#include <dirent.h>

typedef void*      DynamicLib;
static const char* orig_plugin_tail   = "_plugin.so";
static const char* copied_plugin_tail = "_copied.so";
static const int   plugin_ext_len     = 3;

#define LoadDynlib(lib) dlopen(lib, RTLD_NOW)
#define CloseDynlib dlclose
#define GetProc dlsym
#define CopyDynlib(src, dst)                                                                       \
    (!system((string("cp ") + Utils::getPathToExe() + src + " " + Utils::getPathToExe() + dst)     \
                     .c_str()))

#define cast_to_dynlib(x) x

#endif // _WIN32

using namespace std;

static vector<string> getOriginalPlugins();

HA_SCOPED_SINGLETON_IMPLEMENT(PluginManager);

void PluginManager::handleFileAction(FW::WatchID, const FW::String&, const FW::String& filename,
                                     FW::Action action) {
    // if an original plugin .dll has been modified
    if(action == FW::Action::Modified && Utils::endsWith(filename, orig_plugin_tail)) {
        auto plugin_iter =
                find_if(m_plugins.begin(), m_plugins.end(), [&](const LoadedPlugin& curr) {
                    return Utils::endsWith(curr.name_orig, filename);
                });
        // a copy of it should already be loaded
        if(plugin_iter != m_plugins.end()) {
            auto plugin = *plugin_iter;

            std::map<std::string, ObjectJsonMap> mixinPersistence;
            JsonData globalsPersistence;
            globalsPersistence.reserve(1000); // for less allocations for resizing of the array

            {
                // unload
                const auto getMixinsProc = GetProc(cast_to_dynlib(plugin.plugin), "getMixins");
                hassert(getMixinsProc);
                auto& mixins = reinterpret_cast<get_mixins_proc>(getMixinsProc)();
                for(auto& mixin : mixins)
                    mixin.second.unload(mixinPersistence[mixin.first]);

                // serialize the globals
                const auto getGlobalsProc = GetProc(cast_to_dynlib(plugin.plugin), "getGlobals");
                hassert(getGlobalsProc);
                const auto& globals = reinterpret_cast<get_globals_proc>(getGlobalsProc)();
                globalsPersistence.startObject();
                for(auto& global : globals)
                    global.second.serialize(globalsPersistence);
                globalsPersistence.endObject();
            }

            CloseDynlib(cast_to_dynlib(plugin.plugin));
            plugin.plugin = nullptr;

            const auto copy_res = CopyDynlib(plugin.name_orig.c_str(), plugin.name_copy.c_str());
            hassert(copy_res);

            plugin.plugin = LoadDynlib(plugin.name_copy.c_str());
            hassert(plugin.plugin);

            {
                // load
                const auto getMixinsProc = GetProc(cast_to_dynlib(plugin.plugin), "getMixins");
                hassert(getMixinsProc);
                const auto& mixins = reinterpret_cast<get_mixins_proc>(getMixinsProc)();
                for(auto& mixin : mixins) {
                    mixin.second.load(mixinPersistence[mixin.first]);

                    // also update procs by re-registering
                    registerMixin(mixin.first.c_str(), mixin.second);
                }

                // deserialize the globals
                const auto getGlobalsProc = GetProc(cast_to_dynlib(plugin.plugin), "getGlobals");
                hassert(getGlobalsProc);
                const auto& globals         = reinterpret_cast<get_globals_proc>(getGlobalsProc)();
                const sajson::document& doc = globalsPersistence.parse();
                hassert(doc.is_valid());
                const sajson::value& root = doc.get_root();
                for(auto& global : globals) {
                    global.second.deserialize(root);

                    // also update procs by re-registering
                    registerGlobal(global.first.c_str(), global.second);
                }
            }
        }
    }
}

void PluginManager::init() {
    const auto plugin_dlls = getOriginalPlugins();

    for(const auto& curr : plugin_dlls) {
        const string copied   = curr.substr(0, curr.length() - plugin_ext_len) + copied_plugin_tail;
        const auto   copy_res = CopyDynlib(curr.c_str(), copied.c_str());
        hassert(copy_res);

        const auto plugin = LoadDynlib(copied.c_str());
        hassert(plugin);

        // add to the list of registered mixins for the executable
        const auto getMixinsProc = GetProc(plugin, "getMixins");
        hassert(getMixinsProc);
        const auto& mixins = reinterpret_cast<get_mixins_proc>(getMixinsProc)();
        for(auto& mixin : mixins)
            registerMixin(mixin.first.c_str(), mixin.second);

        // register the globals from the plugin to the globals of the executable
        const auto getGlobalsProc = GetProc(plugin, "getGlobals");
        hassert(getGlobalsProc);
        const auto& globals = reinterpret_cast<get_globals_proc>(getGlobalsProc)();
        for(auto& global : globals)
            registerGlobal(global.first.c_str(), global.second);

        m_plugins.push_back({plugin, curr, copied});
    }

    // start the file watcher
    m_fileWatcher.addWatch(Utils::getPathToExe(), this, false);
}

void PluginManager::update() { m_fileWatcher.update(); }

vector<string> getOriginalPlugins() {
    const string spath = Utils::getPathToExe();

    // get all the plugin .dll files near the .exe
    vector<string> names;

#ifdef _WIN32
    const string    search_path = spath + "*" + orig_plugin_tail;
    WIN32_FIND_DATA fd;
    HANDLE          hFind = ::FindFirstFile(search_path.c_str(), &fd);
    if(hFind != INVALID_HANDLE_VALUE) {
        do {
            // read all files in current folder (not directories)
            if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                names.push_back(spath + fd.cFileName);
        } while(::FindNextFile(hFind, &fd));
        ::FindClose(hFind);
    }
#else
    DIR*           dp;
    struct dirent* dirp;
    dp = opendir(spath.c_str());
    hassert(dp);

    while((dirp = readdir(dp)) != nullptr)
        if(Utils::endsWith(dirp->d_name, orig_plugin_tail))
            names.push_back(dirp->d_name);
    closedir(dp);
#endif

    return names;
}

#endif // HA_WITH_PLUGINS
