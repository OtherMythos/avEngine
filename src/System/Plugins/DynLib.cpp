#include "DynLib.h"

#include "Logger/Log.h"

#if _WIN32
    #include <windows.h>
#else
    #include <dlfcn.h>
#endif

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
        if (!std::filesystem::exists(mPluginPath)) {
            return false;
        }

        #if _WIN32
            HMODULE handle = LoadLibraryEx(mPluginPath.c_str(), NULL, 0);
            if(!handle){
                return false;
            }
            void* symbol = GetProcAddress(handle, "dllStartPlugin");
        #else
            void* handle = dlopen(mPluginPath.c_str(), RTLD_NOW);
            if(!handle){
                const char* errorMessage;
                if(errorMessage=dlerror()){
                    AV_ERROR(errorMessage);
                }
                return false;
            }
            void* symbol = dlsym(handle, "dllStartPlugin");
        #endif
        if(!symbol){
            return false;
        }
        void(*funcPoint)(void) = (void(*)(void))symbol;
        (*funcPoint)();

        return true;
    }
}
