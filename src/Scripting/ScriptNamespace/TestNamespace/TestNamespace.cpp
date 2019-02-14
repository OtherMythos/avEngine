#include "TestNamespace.h"

#include "World/WorldSingleton.h"

#include "System/SystemSetup/SystemSettings.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/TestingEvent.h"

#include "TestModeSlotManagerNamespace.h"

namespace AV{
    SQInteger TestNamespace::assertTrue(HSQUIRRELVM vm){
        AV_INFO("assertTrue was called.");
        SQBool assertBool;
        sq_getbool(vm, -1, &assertBool);

        sq_pop(vm, -1);

        if(!assertBool){
            //Send out an event and fail.
            TestingEventBooleanAssertFailed event;
            event.expected = true;

            //EventDispatcher::transmitEvent(EventType::Testing, event);

            sq_throwerror(vm, "Testing error.");
        }

        return 1;
    }

    SQInteger TestNamespace::testModeDisabledMessage(HSQUIRRELVM vm){
        AV_WARN("You called a test mode squirrel function when test mode isn't enabled! "
            "To call these functions you need to enable test mode in the avSetup.cfg file.");

        return 0;
    }

    void TestNamespace::setupNamespace(HSQUIRRELVM vm){
        bool testModeEnabled = SystemSettings::isTestModeEnabled();

        RedirectFunctionMap functionMap;
        functionMap["assertTrue"] = {".b", 2, assertTrue};

        _redirectFunctionMap(vm, testModeDisabledMessage, functionMap, testModeEnabled);

        sq_pushstring(vm, _SC("slotManager"), -1);
        sq_newtable(vm);

        TestModeSlotManagerNamespace smNamespace;
        smNamespace.setupTestNamespace(vm, testModeDisabledMessage, testModeEnabled);

        sq_newslot(vm, -3 , false);

    }
}
