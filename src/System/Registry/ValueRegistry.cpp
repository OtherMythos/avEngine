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
        _setRegistryValue(name, e);
    }

    void ValueRegistry::setIntValue(Ogre::IdString name, int value){
        RegistryEntry e;
        e.t = RegistryType::INT;
        e.i = value;
        _setRegistryValue(name, e);
    }

    void ValueRegistry::setBoolValue(Ogre::IdString name, bool value){
        RegistryEntry e;
        e.t = RegistryType::BOOLEAN;
        e.b = value;
        _setRegistryValue(name, e);
    }

    void ValueRegistry::_setRegistryValue(Ogre::IdString name, RegistryEntry e){
        auto i = mValueMap.find(name);
        if(i == mValueMap.end()) mValueMap[name] = e;
        else{
            auto& ii = (*i).second;
            if(ii.t == RegistryType::STRING){
                _releaseString(ii.s);
            }
            (*i).second = e;
        }
    }

    void ValueRegistry::setStringValue(Ogre::IdString name, const std::string& str){
        auto i = mValueMap.find(name);
        if(i == mValueMap.end()) {
            RegistryEntry e;
            e.t = RegistryType::STRING;
            e.s = _createString(str);
            mValueMap[name] = e;
        }
        else{
            //Check if the other value is a string.
            RegistryEntry& ii = (*i).second;
            if(ii.t == RegistryType::STRING){
                //In this case a simple assignment is possible.
                mStrings[ii.s] = str;
            }else{
                RegistryEntry e;
                e.t = RegistryType::STRING;
                e.s = _createString(str);
                ii = e;
            }
        }
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

    RegistryLookup ValueRegistry::getStringValue(Ogre::IdString name, std::string& outValue){
        //auto i = mStringValueMap.find(name);
        //if(i == mStringValueMap.end()) return REGISTRY_MISSING;
        auto i = mValueMap.find(name);
        if(i == mValueMap.end()) return REGISTRY_MISSING;

        const RegistryEntry& e = (*i).second;

        if(e.t != RegistryType::STRING) return REGISTRY_MISMATCH;
        outValue = mStrings[e.s];

        return REGISTRY_SUCCESS;
    }

    unsigned int ValueRegistry::_createString(const std::string& str){
        if(mAvailableStrings.empty()){
            unsigned int retVal = mStrings.size();
            mStrings.push_back(str);

            return retVal;
        }

        //Otherwise take one of the strings from the free stack.
        unsigned int target = mAvailableStrings.top();
        mAvailableStrings.pop();
        mStrings[target] = str;

        return target;
    }

    void ValueRegistry::_releaseString(unsigned int idx){
        assert(idx < mStrings.size());

        mAvailableStrings.push(idx);
    }
}
