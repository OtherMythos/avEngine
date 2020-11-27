#include "UserComponentManager.h"

#include <cassert>

namespace AV{
    UserComponentManager::UserComponentManager(){

    }

    UserComponentManager::~UserComponentManager(){

    }

    ComponentId _combineTypeAndIdx(ComponentType t, ComponentId i){
        return i | (static_cast<ComponentId>(t) << 62);
    }

    ComponentId _stripIdxFromId(ComponentId i){
        return i & 0x3FFFFFFFFFFFFFFF;
    }

    ComponentType _typeFromId(ComponentId i){
        return (i >> 62) & 0x3;
    }

    ComponentId UserComponentManager::createComponentOfType(ComponentType t){
        //Until I have proper types.
        //00001001
        ComponentCombination targetComb = 0x9;

        ComponentId idx = 0;
        ComponentType compType = 0;
        if(targetComb >= 64) { compType = 3; idx = mComponentVec4.size(); mComponentVec4.push_back({0, 0, 0, 0}); return idx; }
        if(targetComb >= 16) { compType = 2; idx = mComponentVec3.size(); mComponentVec3.push_back({0, 0, 0}); return idx; }
        if(targetComb >= 4) { compType = 1; idx = mComponentVec2.size(); mComponentVec2.push_back({0, 0}); return idx; }
        idx = mComponentVec1.size(); mComponentVec1.push_back({0});

        idx = _combineTypeAndIdx(compType, idx);

        return idx;
    }

    void UserComponentManager::setValue(ComponentId t, uint8 listId, uint8 varIdx, UserComponentDataEntry value){
        UserComponentDataEntry& d = _getDataForList(t, listId, varIdx);
        d = value;
    }

    UserComponentDataEntry UserComponentManager::getValue(ComponentId t, uint8 listId, uint8 varIdx){
        UserComponentDataEntry& d = _getDataForList(t, listId, varIdx);
        return d;
    }

    UserComponentDataEntry& UserComponentManager::_getDataForList(ComponentId t, uint8 listId, uint8 varIdx){
        ComponentId targetId = _stripIdxFromId(t);
        assert(listId < 4);
        switch(listId){
            default:
            case 0: { return mComponentVec1[targetId].e; }
            case 1: { assert(varIdx < 2); return mComponentVec2[targetId].e[listId]; }
            case 2: { assert(varIdx < 3); return mComponentVec3[targetId].e[listId]; }
            case 3: { assert(varIdx < 4); return mComponentVec4[targetId].e[listId]; }
        }
    }

    ComponentCombination UserComponentManager::_dataTypesToCombination(const ComponentDataTypes (&data)[4]){
        ComponentCombination c = 0;

        for(uint8 i = 0; i < MAX_COMPONENT_DATA_TYPES; i++){
            ComponentDataTypes d = data[i];
            c |= static_cast<unsigned char>(d) << (i * 2);
        }
        return c;
    }

    void UserComponentManager::_combinationToDataTypes(ComponentCombination data, ComponentDataTypes (&outData)[MAX_COMPONENT_DATA_TYPES]){
        for(uint8 i = 0; i < MAX_COMPONENT_DATA_TYPES; i++){
            ComponentDataTypes d = static_cast<ComponentDataTypes>( (data >> i * 2) & 0x3 );
            outData[i] = d;
        }
    }
}
