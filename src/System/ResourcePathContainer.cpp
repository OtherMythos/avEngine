#include "ResourcePathContainer.h"

namespace AV {
    std::string ResourcePathContainer::_resourcePath = "";
    
    const std::string& ResourcePathContainer::getResourcePath(){
        return _resourcePath;
    }
    
    void ResourcePathContainer::setResourcePath(const std::string &path) {
        _resourcePath = path;
    }
}
