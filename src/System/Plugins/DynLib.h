#pragma once

#include <string>

namespace AV{
    class DynLib{
    public:
        DynLib(const std::string& pluginName, const std::string& pluginPath);
        ~DynLib();

        bool load();

    private:
        std::string mPluginName;
        std::string mPluginPath;
        void* mLibHandle;
    };
}
