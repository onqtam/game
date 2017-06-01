#pragma once

#ifndef EMSCRIPTEN

#include "utils/singleton.h"

HARDLY_SUPPRESS_WARNINGS
#include <FileWatcher/FileWatcher.h>
HARDLY_SUPPRESS_WARNINGS_END

class PluginManager : public FW::FileWatchListener
{
    HARDLY_SCOPED_SINGLETON(PluginManager, class ExampleHelloWorld);

private:
    struct LoadedPlugin
    {
        void*       plugin;
        std::string name_orig;
        std::string name_copy;
    };

    FW::FileWatcher fileWatcher;

    std::vector<LoadedPlugin> plugins;

public:
    void handleFileAction(FW::WatchID watchid, const FW::String& dir, const FW::String& filename,
                          FW::Action action) override;

    void init();
    void update();
};

#endif // EMSCRIPTEN
