#include "UserComponentManager.h"

#include <cassert>
#include "System/SystemSetup/SystemSettings.h"

namespace AV{

    UserComponentManager::UserComponentManager(){

    }

    UserComponentManager::~UserComponentManager(){

    }

    #define CREATE_IN_LIST(LIST) {\
    for(size_t i = 0; i < LIST.size(); i++){ \
        if(!LIST[i].populated) { LIST[i] = LIST_VALUE; return i; } \
    } \
    idx = LIST.size(); \
    LIST.push_back(LIST_VALUE); \
    return idx; }\


    ComponentId UserComponentManager::createComponentOfType(ComponentType t){
        const UserComponentSettings::ComponentSetting& settings = SystemSettings::getUserComponentSettings().vars[t];

        mNumRegisteredComponents++;
        ComponentId idx = 0;
        switch(settings.numVars){
            case 1:
                #define LIST_VALUE { true, {0} }
                    CREATE_IN_LIST(mComponentVec1)
                #undef LIST_VALUE
            case 2:
                #define LIST_VALUE { true, {0, 0} }
                    CREATE_IN_LIST(mComponentVec2)
                #undef LIST_VALUE
            case 3:
                #define LIST_VALUE { true, {0, 0, 0} }
                    CREATE_IN_LIST(mComponentVec3)
                #undef LIST_VALUE
            case 4:
                #define LIST_VALUE { true, {0, 0, 0, 0} }
                    CREATE_IN_LIST(mComponentVec4)
                #undef LIST_VALUE
            default:
                assert(false);
                break;
        }

        return idx;
    }

    void UserComponentManager::removeComponent(ComponentId id, ComponentType t){
        const UserComponentSettings::ComponentSetting& settings = SystemSettings::getUserComponentSettings().vars[t];

        switch(settings.numVars){
            case 1:
                mComponentVec1[id].populated = false;
                break;
            case 2:
                mComponentVec2[id].populated = false;
                break;
            case 3:
                mComponentVec3[id].populated = false;
                break;
            case 4:
                mComponentVec4[id].populated = false;
                break;
            default:
                assert(false);
                break;
        }
        mNumRegisteredComponents--;
    }

    void UserComponentManager::setValue(ComponentId t, ComponentType compType, uint8 varIdx, UserComponentDataEntry value){
        UserComponentDataEntry& d = _getDataForList(t, compType, varIdx);
        d = value;
    }

    UserComponentDataEntry UserComponentManager::getValue(ComponentId t, ComponentType compType, uint8 varIdx){
        UserComponentDataEntry& d = _getDataForList(t, compType, varIdx);
        return d;
    }

    UserComponentDataEntry& UserComponentManager::_getDataForList(ComponentId targetId, ComponentType compType, uint8 varIdx){
        uint8 listId = SystemSettings::getUserComponentSettings().vars[compType].numVars;
        assert(listId > 0 && listId <= 4);
        switch(listId){
            default:
            case 1: { return mComponentVec1[targetId].e; }
            case 2: { assert(varIdx < 2); return mComponentVec2[targetId].e[varIdx]; }
            case 3: { assert(varIdx < 3); return mComponentVec3[targetId].e[varIdx]; }
            case 4: { assert(varIdx < 4); return mComponentVec4[targetId].e[varIdx]; }
        }
    }
}
