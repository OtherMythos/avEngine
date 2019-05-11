#pragma once

#include "World/Entity/eId.h"
#include <string>

#include <vector>

namespace AV{
    /**
    A class to store strings necessary for component deserialisation.
    
    Some of the work of the deseraliser needs to be performed on the main thread.
    A worker thread can write into a class like this any string information that's necessary to do this.
    */
    class SerialiserStringStore{
    public:
        SerialiserStringStore();
        ~SerialiserStringStore();
        
        typedef std::pair<eId, std::string> StringEntry;
        
        std::vector<StringEntry> mStoredStrings;
        
        bool ready = false;
    };
}
