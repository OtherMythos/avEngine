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
          mPluginPath(pluginPath),
          mLibHandle(nullptr) {

    }

    DynLib::~DynLib(){
    #if _WIN32
        if(mLibHandle){
            FreeLibrary(mLibHandle);
            mLibHandle = nullptr;
        }
    #else
        if(mLibHandle){
            dlclose(mLibHandle);
            mLibHandle = nullptr;
        }
    #endif
    }

    bool DynLib::load(){
        if (!std::filesystem::exists(mPluginPath)) {
            return false;
        }

        #if _WIN32
            mLibHandle = LoadLibraryEx(mPluginPath.c_str(), NULL, 0);
            if(!mLibHandle){
                return false;
            }
            void* symbol = GetProcAddress(mLibHandle, "dllStartPlugin");
        #else
            mLibHandle = dlopen(mPluginPath.c_str(), RTLD_NOW);
            if(!mLibHandle){
                const char* errorMessage;
                if(errorMessage=dlerror()){
                    AV_ERROR(errorMessage);
                }
                return false;
            }
            void* symbol = dlsym(mLibHandle, "dllStartPlugin");
        #endif
        if(!symbol){
            return false;
        }
        void(*funcPoint)(void) = (void(*)(void))symbol;
        (*funcPoint)();

        return true;
    }
}
