#pragma once

#include <string>

namespace AV{
    class ResourcePathContainer{
    private:
        static std::string _resourcePath;
    
    public:
        static const std::string& getResourcePath();
        static void setResourcePath(const std::string &path);
    };
}
