#pragma once

#include "framework/IEscapeRoom.h"
#include <string>
#include <vector>
#include <memory>

namespace devescape {

struct PluginInfo {
    std::string name;
    std::string path;
    std::string version;
    std::string author;
    std::string description;
    void* handle;  // Library handle
};

class DEVESCAPE_API PluginManager {
public:
    PluginManager(const std::string& pluginDirectory);
    ~PluginManager();

    // Plugin discovery and loading
    void scanForPlugins();
    std::vector<PluginInfo> getAvailablePlugins() const;

    // Room loading
    IEscapeRoom* loadRoom(const std::string& pluginName);
    void unloadRoom(IEscapeRoom* room, const std::string& pluginName);

private:
    std::string pluginDirectory_;
    std::vector<PluginInfo> plugins_;

    bool loadPlugin(const std::string& path, PluginInfo& info);
    void unloadPlugin(PluginInfo& info);

#ifdef _WIN32
    void* loadLibrary(const std::string& path);
    void* getFunction(void* handle, const char* name);
#else
    void* loadLibrary(const std::string& path);
    void* getFunction(void* handle, const char* name);
#endif
};

} // namespace devescape
