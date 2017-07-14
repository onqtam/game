#pragma once

#ifdef HA_WITH_PLUGINS

HA_SUPPRESS_WARNINGS
#include <FileWatcher/FileWatcher.h>
HA_SUPPRESS_WARNINGS_END

class PluginManager : public FW::FileWatchListener, public Singleton<PluginManager>
{
    HA_SINGLETON(PluginManager);
    PluginManager()
            : Singleton(this) {}
    friend class Application;

    struct LoadedPlugin
    {
        void*       plugin;
        std::string name_orig;
        std::string name_copy;
    };

    FW::FileWatcher m_fileWatcher;

    std::vector<LoadedPlugin> m_plugins;

    void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename,
                          FW::Action action) override;
    void init();
    void update();
};

#endif // HA_WITH_PLUGINS
