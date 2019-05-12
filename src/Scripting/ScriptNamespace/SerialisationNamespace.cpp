#include "SerialisationNamespace.h"

#include "Serialisation/SaveHandle.h"
#include "Serialisation/SerialisationManager.h"
#include "Scripting/ScriptNamespace/Classes/SaveHandleClass.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

#include "System/BaseSingleton.h"
#include "System/SystemSetup/SystemSettings.h"

namespace AV{
    SQInteger SerialisationNamespace::getAvailableSaves(HSQUIRRELVM vm){
        //Returns an array of save handles.
        BaseSingleton::getSerialisationManager()->scanForSaves();
        const std::vector<SaveHandle>& saves = BaseSingleton::getSerialisationManager()->getAvailableSaves();
        
        sq_newarray(vm, 0);
        for(const SaveHandle& s : saves){
            ScriptUtils::_debugStack(vm);
            SaveHandleClass::saveHandleToInstance(vm, s);
            ScriptUtils::_debugStack(vm);
            sq_arrayappend(vm, -2);
        }

        return 1;
    }

    void SerialisationNamespace::setupNamespace(HSQUIRRELVM vm){
        _addFunction(vm, getAvailableSaves, "getAvailableSaves");
    }

}
