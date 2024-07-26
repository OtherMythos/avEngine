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
    };

}
