#pragma once

#include <string>

#if _WIN32
    #include <windows.h>
#endif

namespace AV{
    class DynLib{
    public:
        DynLib(const std::string& pluginName, const std::string& pluginPath);
        ~DynLib();

        bool load();

    private:
        std::string mPluginName;
        std::string mPluginPath;

        #if _WIN32
            HMODULE mLibHandle = nullptr;
        #else
            void* mLibHandle = nullptr;
        #endif
    };
}
