#include "ValueRegistry.h"

namespace AV{
    ValueRegistry::ValueRegistry(){

    }

    ValueRegistry::~ValueRegistry(){

    }

    void ValueRegistry::setFloatValue(Ogre::IdString name, float value){
        RegistryEntry e;
        e.t = RegistryType::FLOAT;
        e.f = value;
        mValueMap[name] = e;
    }

    void ValueRegistry::setIntValue(Ogre::IdString name, int value){
        RegistryEntry e;
        e.t = RegistryType::INT;
        e.i = value;
        mValueMap[name] = e;
    }

    void ValueRegistry::setBoolValue(Ogre::IdString name, bool value){
        RegistryEntry e;
        e.t = RegistryType::BOOLEAN;
        e.b = value;
        mValueMap[name] = e;
    }

    RegistryLookup ValueRegistry::getFloatValue(Ogre::IdString name, float& outValue){
        auto i = mValueMap.find(name);
        if(i == mValueMap.end()) return REGISTRY_MISSING;

        const RegistryEntry& e = (*i).second;

        if(e.t != RegistryType::FLOAT) return REGISTRY_MISMATCH;
        outValue = e.f;

        return REGISTRY_SUCCESS;
    }

    RegistryLookup ValueRegistry::getIntValue(Ogre::IdString name, int& outValue){
        auto i = mValueMap.find(name);
        if(i == mValueMap.end()) return REGISTRY_MISSING;

        const RegistryEntry& e = (*i).second;

        if(e.t != RegistryType::INT) return REGISTRY_MISMATCH;
        outValue = e.i;

        return REGISTRY_SUCCESS;
    }

    RegistryLookup ValueRegistry::getBoolValue(Ogre::IdString name, bool& outValue){
        auto i = mValueMap.find(name);
        if(i == mValueMap.end()) return REGISTRY_MISSING;

        const RegistryEntry& e = (*i).second;

        if(e.t != RegistryType::BOOLEAN) return REGISTRY_MISMATCH;
        outValue = e.b;

        return REGISTRY_SUCCESS;
    }
}
