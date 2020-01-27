#pragma once

#include <OgreIdString.h>
#include <map>
#include <vector>
#include <stack>

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
            unsigned int s;
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
        void setStringValue(Ogre::IdString name, const std::string& str);

        RegistryLookup getFloatValue(Ogre::IdString name, float& outValue);
        RegistryLookup getIntValue(Ogre::IdString name, int& outValue);
        RegistryLookup getBoolValue(Ogre::IdString name, bool& outValue);
        RegistryLookup getStringValue(Ogre::IdString name, std::string& outValue);
        RegistryLookup getValue(Ogre::IdString name, const void*& v, RegistryType& t);

        /**
        Clear all values from the registry.
        */
        void clear();

    private:
        std::map<Ogre::IdString, RegistryEntry> mValueMap;

        std::vector<std::string> mStrings;
        std::stack<unsigned int> mAvailableStrings;

        unsigned int _createString(const std::string& str);
        void _releaseString(unsigned int idx);

        inline void _setRegistryValue(Ogre::IdString name, RegistryEntry e);
    };
}
