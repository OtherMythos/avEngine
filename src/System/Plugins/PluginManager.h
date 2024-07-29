#pragma once

#include <vector>
#include <string>

namespace AV{

    class DynLib;
    class Plugin;

    /**
    Singleton plugin manager for custom native code execution.
    */
    class PluginManager{
    public:
        PluginManager() = delete;
        ~PluginManager() = delete;

        static void initialise();

        static void registerPlugin(Plugin* plugin);
        static void loadPlugin(const std::string& pluginName, const std::string& pluginPath);

    private:
        static std::vector<DynLib*> mDynamicLibs;
        static std::vector<Plugin*> mPlugins;

        static void _loadDynamicPlugins();
        static std::string _findPluginForName(const std::string& basePath, const std::string& pluginName);
        static void _scanToLoadPlugin(const std::string& pluginName, const std::vector<std::string>& pluginPaths);

        static const char* _getFileExtension();
        static const char* _getCPUArchitecture();
        static const char* _getPlatform();


    };

}
