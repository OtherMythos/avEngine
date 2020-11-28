#include "UserComponentManager.h"

#include <cassert>
#include "System/SystemSetup/SystemSettings.h"

namespace AV{

    UserComponentManager::UserComponentManager(){

    }

    UserComponentManager::~UserComponentManager(){

    }

    ComponentId UserComponentManager::createComponentOfType(ComponentType t){
        const UserComponentSettings::ComponentSetting& settings = SystemSettings::getUserComponentSettings().vars[t];

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
        uint8 listId = SystemSettings::getUserComponentSettings().vars[compType].numVars;
        assert(listId > 0 && listId <= 4);
        switch(listId){
            default:
            case 1: { return mComponentVec1[targetId].e; }
            case 2: { assert(varIdx < 2); return mComponentVec2[targetId].e[listId]; }
            case 3: { assert(varIdx < 3); return mComponentVec3[targetId].e[listId]; }
            case 4: { assert(varIdx < 4); return mComponentVec4[targetId].e[listId]; }
        }
    }
}
