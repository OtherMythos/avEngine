#include "TestModeSlotManagerNamespace.h"

#include "Logger/Log.h"
#include "World/WorldSingleton.h"
#include "World/Slot/SlotManager.h"

namespace AV{
    SQInteger TestModeSlotManagerNamespace::getQueueSize(HSQUIRRELVM vm){
        World* world = WorldSingleton::getWorld();
        if(world){
            SQInteger size = world->getSlotManager()->queuedEntries.size();
            sq_pushinteger(vm, size);
            return 1;
        }
        return 0;
    }

    void TestModeSlotManagerNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        RedirectFunctionMap functionMap;
        functionMap["getQueueSize"] = {"", 0, getQueueSize};

        _redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}
