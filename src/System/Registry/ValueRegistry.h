#pragma once

#include <OgreIdString.h>
#include <map>
#include <vector>

namespace AV{

    enum class RegistryType{
        STRING,
        FLOAT,
        INT,
        BOOLEAN
    };

    enum RegistryLookup{
        REGISTRY_SUCCESS = 1,
        REGISTRY_MISSING,
        REGISTRY_MISMATCH //type mismatch, i.e asking for an int and finding a float.
    };

    struct RegistryEntry{
        RegistryType t;

        union{
            int i;
            float f;
            bool b;
        };
    };

    inline bool lookupSuccess(RegistryLookup l){
        return l == REGISTRY_SUCCESS;
    }

    /**
    A class to manage generic values.
    */
    class ValueRegistry{
    public:
        ValueRegistry();
        ~ValueRegistry();

        void setFloatValue(Ogre::IdString name, float value);
        void setIntValue(Ogre::IdString name, int value);
        void setBoolValue(Ogre::IdString name, bool value);

        RegistryLookup getFloatValue(Ogre::IdString name, float& outValue);
        RegistryLookup getIntValue(Ogre::IdString name, int& outValue);
        RegistryLookup getBoolValue(Ogre::IdString name, bool& outValue);

    private:
        std::map<Ogre::IdString, RegistryEntry> mValueMap;

        std::vector<std::string> mStrings;
    };
}
