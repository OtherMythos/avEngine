#pragma once

#include "System/Util/IdString.h"
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
    These values are stored as key-value-pairs, and can be a number of types.
    */
    class ValueRegistry{
    public:
        ValueRegistry();
        ~ValueRegistry();

        void setFloatValue(IdString name, float value);
        void setIntValue(IdString name, int value);
        void setBoolValue(IdString name, bool value);
        void setStringValue(IdString name, const std::string& str);

        RegistryLookup getFloatValue(IdString name, float& outValue);
        RegistryLookup getIntValue(IdString name, int& outValue);
        RegistryLookup getBoolValue(IdString name, bool& outValue);
        RegistryLookup getStringValue(IdString name, std::string& outValue);
        RegistryLookup getValue(IdString name, const void*& v, RegistryType& t);

        /**
        Remove an entry from the registry.
        */
        void removeValue(IdString name);

        /**
        Clear all values from the registry.
        */
        void clear();

        const char* getStringTypeOfEntry(IdString id) const;

        /**
        Records the original string name of a key so it can be later retrieved via getKeys.
        Should be called alongside set*Value whenever the original string name is available.
        */
        void trackKey(const std::string& name);

        /**
        Returns a list of all the currently tracked key names in the registry.
        Only keys that were tracked via trackKey will appear here.
        */
        std::vector<std::string> getKeys() const;

    private:
        std::map<IdString, RegistryEntry> mValueMap;
        std::map<IdString, std::string> mKeyNames;

        std::vector<std::string> mStrings;
        std::stack<unsigned int> mAvailableStrings;

        unsigned int _createString(const std::string& str);
        void _releaseString(unsigned int idx);

        inline void _setRegistryValue(IdString name, RegistryEntry e);
    };
}
