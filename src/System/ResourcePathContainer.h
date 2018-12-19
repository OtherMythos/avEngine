#pragma once

namespace AV{
    class ResourcePathContainer{
    private:
        static std::string _resourcePath;
    
    public:
        static const std::string& getResourcePath() { return _resourcePath; };
        static void setResourcePath(const std::string &path) { _resourcePath = path; };
    };
    
    std::string ResourcePathContainer::_resourcePath = "";
}
