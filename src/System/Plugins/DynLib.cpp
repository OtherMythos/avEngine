#include "DynLib.h"

#include <dlfcn.h>
#include <filesystem>
#include "System/Util/PathUtils.h"

namespace AV{
    DynLib::DynLib(const std::string& pluginName, const std::string& pluginPath)
        : mPluginName(pluginName),
          mPluginPath(pluginPath) {

    }

    DynLib::~DynLib(){

    }

    bool DynLib::load(){
        std::string outString;
        formatResToPath(mPluginPath, outString);

        std::filesystem::path base(outString);
        std::filesystem::path tested = base / (mPluginName + ".so");

        void* handle = dlopen(tested.string().c_str(), RTLD_NOW);
        if(!handle){
            return false;
        }
        void* symbol = dlsym(handle, "dllStartPlugin");
        if(!symbol){
            return false;
        }
        void(*funcPoint)(void) = (void(*)(void))symbol;
        (*funcPoint)();

        return true;
    }
}
