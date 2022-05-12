#include "SerialisationNamespace.h"

#include "Serialisation/SaveHandle.h"
#include "Serialisation/SerialisationManager.h"
#include "Scripting/ScriptNamespace/Classes/SaveHandleClass.h"

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
            //NOTE these need to be here so the compiler doesn't optimise them out. TODO find them a proper home.
            ScriptUtils::_debugStack(vm);
            ScriptUtils::_debugBacktrace(vm);
            sq_arrayappend(vm, -2);
        }

        return 1;
    }

    SQInteger SerialisationNamespace::clearAllSaves(HSQUIRRELVM vm){
        BaseSingleton::getSerialisationManager()->clearAllSaves();

        return 0;
    }

    void SerialisationNamespace::setupNamespace(HSQUIRRELVM vm){
        ScriptUtils::addFunction(vm, getAvailableSaves, "getAvailableSaves");
        ScriptUtils::addFunction(vm, clearAllSaves, "clearAllSaves");
    }

}
