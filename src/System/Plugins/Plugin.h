#pragma once

#include <string>

namespace AV{

    class Plugin{
    public:
        Plugin(const std::string& pluginName);
        ~Plugin();

        const std::string& getName() { return mName; };

        virtual void initialise();
        virtual void shutdown();

    private:
        std::string mName;
    };

}
