#include "PluginManager.h"

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"

#include "DynLib.h"
#include "Plugin.h"

namespace AV{
    std::vector<DynLib*> PluginManager::mDynamicLibs;
    std::vector<Plugin*> PluginManager::mPlugins;

    void PluginManager::initialise(){
        for(const SystemSettings::PluginEntry& e : SystemSettings::getPluginEntries()){
            loadPlugin(e.name, e.path);
        }
    }

    void PluginManager::loadPlugin(const std::string& pluginName, const std::string& pluginPath){
        DynLib* lib = new DynLib(pluginName, pluginPath);
        bool result = lib->load();
        if(!result){
            AV_ERROR("Error loading plugin '{}'", pluginName);
        }

        mDynamicLibs.push_back(lib);
    }

    void PluginManager::registerPlugin(Plugin* plugin){
        mPlugins.push_back(plugin);
        AV_INFO("Registering plugin with name '{}'", plugin->getName());
        plugin->initialise();
        AV_INFO("Initialise plugin '{}'", plugin->getName());
    }

}
