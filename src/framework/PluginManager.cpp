#include "framework/PluginManager.h"
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace fs = std::filesystem;

namespace devescape {

PluginManager::PluginManager(const std::string& pluginDirectory)
    : pluginDirectory_(pluginDirectory) {
    fs::create_directories(pluginDirectory);
}

PluginManager::~PluginManager() {
    for (auto& plugin : plugins_) {
        unloadPlugin(plugin);
    }
}

void PluginManager::scanForPlugins() {
    plugins_.clear();

    try {
        for (const auto& entry : fs::directory_iterator(pluginDirectory_)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();

#ifdef _WIN32
                if (ext == ".dll") {
#else
                if (ext == ".so") {
#endif
                    PluginInfo info;
                    if (loadPlugin(entry.path().string(), info)) {
                        plugins_.push_back(info);
                        std::cout << "Loaded plugin: " << info.name << std::endl;
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error scanning plugins: " << e.what() << std::endl;
    }
}

std::vector<PluginInfo> PluginManager::getAvailablePlugins() const {
    return plugins_;
}

bool PluginManager::loadPlugin(const std::string& path, PluginInfo& info) {
    void* handle = loadLibrary(path);
    if (!handle) {
        std::cerr << "Failed to load plugin: " << path << std::endl;
        return false;
    }

    // Get required function pointers
    typedef const char* (*GetNameFunc)();
    typedef uint32_t (*GetVersionFunc)();

    auto getName = reinterpret_cast<GetNameFunc>(getFunction(handle, "getPluginName"));
    auto getVersion = reinterpret_cast<GetVersionFunc>(getFunction(handle, "getPluginVersion"));

    if (!getName || !getVersion) {
        std::cerr << "Plugin missing required exports: " << path << std::endl;
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle));
#else
        dlclose(handle);
#endif
        return false;
    }

    info.name = getName();
    info.path = path;
    info.version = std::to_string(getVersion());
    info.handle = handle;
    info.author = "Unknown";
    info.description = "Escape room plugin";

    return true;
}

void PluginManager::unloadPlugin(PluginInfo& info) {
    if (info.handle) {
#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(info.handle));
#else
        dlclose(info.handle);
#endif
        info.handle = nullptr;
    }
}

IEscapeRoom* PluginManager::loadRoom(const std::string& pluginName) {
    for (const auto& plugin : plugins_) {
        if (plugin.name == pluginName) {
            typedef IEscapeRoom* (*CreateRoomFunc)();
            auto createRoom = reinterpret_cast<CreateRoomFunc>(getFunction(plugin.handle, "createRoom"));

            if (createRoom) {
                return createRoom();
            }
        }
    }

    std::cerr << "Plugin not found: " << pluginName << std::endl;
    return nullptr;
}

void PluginManager::unloadRoom(IEscapeRoom* room, const std::string& pluginName) {
    for (const auto& plugin : plugins_) {
        if (plugin.name == pluginName) {
            typedef void (*DestroyRoomFunc)(IEscapeRoom*);
            auto destroyRoom = reinterpret_cast<DestroyRoomFunc>(getFunction(plugin.handle, "destroyRoom"));

            if (destroyRoom && room) {
                destroyRoom(room);
            }
            return;
        }
    }
}

#ifdef _WIN32
void* PluginManager::loadLibrary(const std::string& path) {
    return LoadLibraryA(path.c_str());
}

void* PluginManager::getFunction(void* handle, const char* name) {
    return GetProcAddress(static_cast<HMODULE>(handle), name);
}
#else
void* PluginManager::loadLibrary(const std::string& path) {
    return dlopen(path.c_str(), RTLD_LAZY);
}

void* PluginManager::getFunction(void* handle, const char* name) {
    return dlsym(handle, name);
}
#endif

} // namespace devescape
