#include "UserComponentManager.h"

#include <cassert>

namespace AV{
    UserComponentSettings UserComponentManager::mSettings;

    UserComponentManager::UserComponentManager(){
        //TODO temporary
        {
            mSettings.vars[0].componentName = "health";
            ComponentDataTypes list[4] = {ComponentDataTypes::INT, ComponentDataTypes::NONE, ComponentDataTypes::NONE, ComponentDataTypes::NONE,};
            ComponentCombination comb = _dataTypesToCombination(list);
            mSettings.vars[0].componentVars = comb;
            mSettings.vars[0].numVars = 1;
        }

        {
            mSettings.vars[1].componentName = "otherHealth";
            ComponentDataTypes list[4] = {ComponentDataTypes::INT, ComponentDataTypes::FLOAT, ComponentDataTypes::NONE, ComponentDataTypes::NONE,};
            ComponentCombination comb = _dataTypesToCombination(list);
            mSettings.vars[1].componentVars = comb;
            mSettings.vars[1].numVars = 2;
        }
        mSettings.numRegisteredComponents = 2;
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
        const UserComponentSettings::ComponentSetting& settings = mSettings.vars[t];

        ComponentId idx = 0;
        switch(settings.numVars){
            case 1:
                idx = mComponentVec1.size();
                mComponentVec1.push_back({0});
                break;
            case 2:
                idx = mComponentVec2.size();
                mComponentVec2.push_back({0, 0});
                break;
            case 3:
                idx = mComponentVec3.size();
                mComponentVec3.push_back({0, 0, 0});
                break;
            case 4:
                idx = mComponentVec4.size();
                mComponentVec4.push_back({0, 0, 0, 0});
                break;
            default:
                assert(false);
                break;
        }

        idx = _combineTypeAndIdx(settings.numVars, idx);

        return idx;
    }

    void UserComponentManager::setValue(ComponentId t, ComponentType compType, uint8 varIdx, UserComponentDataEntry value){
        UserComponentDataEntry& d = _getDataForList(t, compType, varIdx);
        d = value;
    }

    UserComponentDataEntry UserComponentManager::getValue(ComponentId t, ComponentType compType, uint8 varIdx){
        UserComponentDataEntry& d = _getDataForList(t, compType, varIdx);
        return d;
    }

    UserComponentDataEntry& UserComponentManager::_getDataForList(ComponentId t, ComponentType compType, uint8 varIdx){
        ComponentId targetId = _stripIdxFromId(t);
        uint8 listId = mSettings.vars[compType].numVars;
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
            outData[i] = mSettings.getTypeOfVariable(data, i);
        }
    }
}
