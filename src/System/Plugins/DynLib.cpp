#include "DynLib.h"

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
        std::string outString;
        formatResToPath(mPluginPath, outString);

        std::filesystem::path base(outString);
        const char* fileExtension =
#ifdef WIN32
            ".dll"
#else
            ".so"
#endif
            ;
        std::filesystem::path tested = base / (mPluginName + fileExtension);

        if (!std::filesystem::exists(tested)) {
            return false;
        }

        #if _WIN32
            HMODULE handle = LoadLibraryEx(tested.string().c_str(), NULL, 0);
            if(!handle){
                return false;
            }
            void* symbol = GetProcAddress(handle, "dllStartPlugin");
        #else
            void* handle = dlopen(tested.string().c_str(), RTLD_NOW);
            if(!handle){
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
