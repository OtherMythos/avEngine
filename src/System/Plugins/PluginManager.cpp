#include "PluginManager.h"

#include "Logger/Log.h"
#include "System/SystemSetup/SystemSettings.h"

#ifdef USE_STATIC_PLUGINS
    #define REGISTER_PLUGIN(xx, yy) { yy* p = new yy(); AV::PluginManager::registerPlugin(p); }
    #include "StaticPlugins.h"
#endif

#include "DynLib.h"
#include "Plugin.h"

#include "System/Util/PathUtils.h"

namespace AV{
    std::vector<DynLib*> PluginManager::mDynamicLibs;
    std::vector<Plugin*> PluginManager::mPlugins;

    void PluginManager::initialise(){
#ifdef USE_STATIC_PLUGINS
        AV_INFO("Loading static plugins");
        registerStaticPlugins();
        AV_INFO("End load static plugins");
#else
    #ifndef TARGET_APPLE_IPHONE
        _loadDynamicPlugins();
    #endif
#endif
    }

    void PluginManager::loadPlugin(const std::string& pluginName, const std::string& pluginPath){
        AV_INFO("Loading plugin at path {}", pluginPath);
        DynLib* lib = new DynLib(pluginName, pluginPath);
        bool result = lib->load();
        if(!result){
            AV_ERROR("Error loading plugin '{}' at path '{}'", pluginName, pluginPath);
        }

        mDynamicLibs.push_back(lib);
    }

    void PluginManager::registerPlugin(Plugin* plugin){
        mPlugins.push_back(plugin);
        AV_INFO("Registering plugin with name '{}'", plugin->getName());
        plugin->initialise();
        AV_INFO("Initialise plugin '{}'", plugin->getName());
    }

    void PluginManager::_loadDynamicPlugins(){
        if(!SystemSettings::getPluginEntries().empty()){
            AV_INFO("Loading dynamic plugins");
            for(const SystemSettings::PluginEntry& e : SystemSettings::getPluginEntries()){
                _scanToLoadPlugin(e.name, e.paths);
            }
            AV_INFO("End load dynamic plugins");
        }
    }

    void PluginManager::_scanToLoadPlugin(const std::string& pluginName, const std::vector<std::string>& pluginPaths){
        std::string pluginPath = "";
        for(const std::string& p : pluginPaths){
            pluginPath = _findPluginForName(p, pluginName);
            if(!pluginPath.empty()) break;
        }
        if(pluginPath.empty()){
            AV_ERROR("Unable to find plugin with name '{}'", pluginName);
            return;
        }

        loadPlugin(pluginName, pluginPath);
    }

    const char* PluginManager::_getFileExtension(){
        #ifdef WIN32
            return ".dll";
        #else
            return ".so";
        #endif
    }
    const char* PluginManager::_getCPUArchitecture(){
        #ifdef __APPLE__
            #ifdef __arm64__
                return "arm64";
            #else
                return "x86_64";
            #endif
        #else
            return "x86_64";
        #endif
    }
    const char* PluginManager::_getPlatform(){
        #ifdef _WIN32
            return "windows";
        #elif __APPLE__
            #ifdef TARGET_APPLE_IPHONE
                return "ios";
            #else
                return "macos";
            #endif
        #else
            return "linux";
        #endif
    }

    std::string PluginManager::_findPluginForName(const std::string& basePath, const std::string& pluginName){
        std::string outString;
        AV::formatResToPath(basePath, outString);

        const std::filesystem::path targetBasePath(outString);
        if(!std::filesystem::exists(targetBasePath)){
            return "";
        }
        if(std::filesystem::is_regular_file(targetBasePath)){
            //If the path itself is a direct path to a file then just take that, assuming it has the correct extension.
            if(targetBasePath.extension().string() == _getFileExtension()){
                return std::filesystem::canonical(targetBasePath).string();
            }
        }

        std::filesystem::directory_iterator it(targetBasePath);
        for(const std::filesystem::path& path : it){
            //If the extension matches the file type then check it for the types.
            if(path.extension().string() != _getFileExtension()) continue;

            const std::string filename = path.filename().string();
            if(filename.find(pluginName) != 0 && filename.find("lib" + pluginName) != 0) continue;
            if(filename.find(_getCPUArchitecture()) == std::string::npos) continue;
            if(filename.find(_getPlatform()) == std::string::npos) continue;

            if(!std::filesystem::exists(path) || !(std::filesystem::is_regular_file(path) || std::filesystem::is_symlink(path))){
                continue;
            }
            //This being the first paths that matched completely, just use this.
            return std::filesystem::canonical(path).string();
        }

        return "";
    }

}
