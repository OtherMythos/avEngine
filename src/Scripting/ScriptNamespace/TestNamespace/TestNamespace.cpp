#ifdef TEST_MODE

#include "TestNamespace.h"

#include <iostream>
#include <fstream>
#include "World/WorldSingleton.h"

#include "System/SystemSetup/SystemSettings.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/TestingEvent.h"

#include "TestModeSlotManagerNamespace.h"
#include "TestModeEntityManagerNamespace.h"
#include "TestModeSerialisationNamespace.h"
#include "TestModePhysicsNamespace.h"
#include "TestModeTextureNamespace.h"
#include "TestModeInputNamespace.h"
#include "TestModeGuiNamespace.h"
#include "TestModeUserComponentNamespace.h"
#include "Scripting/ScriptNamespace/ScriptUtils.h"

namespace AV{
    SQInteger TestNamespace::assertTrue(HSQUIRRELVM vm){
        return _processBooleanAssert(vm, true);
    }

    SQInteger TestNamespace::assertFalse(HSQUIRRELVM vm){
        return _processBooleanAssert(vm, false);
    }

    SQInteger TestNamespace::assertEqual(HSQUIRRELVM vm){
        return _processComparisonAssert(vm, true);
    }

    SQInteger TestNamespace::assertNotEqual(HSQUIRRELVM vm){
        return _processComparisonAssert(vm, false);
    }


    SQInteger TestNamespace::endTest(HSQUIRRELVM vm){
        SQObjectType type = sq_gettype(vm, -1);
        SQBool testEndSuccess = true;
        if(type == OT_BOOL){
            sq_getbool(vm, -1, &testEndSuccess);
        }
        TestingEventTestEnd event;
        event.successfulEnd = testEndSuccess;

        SQStackInfos si;
        sq_stackinfos(vm, 1, &si);
        event.lineNum = si.line;
        event.srcFile = si.source;

        EventDispatcher::transmitEvent(EventType::Testing, event);

        return SQ_ERROR;
    }

    SQInteger TestNamespace::_processBooleanAssert(HSQUIRRELVM vm, bool intendedResult){
        SQBool assertBool;
        sq_getbool(vm, -1, &assertBool);
        sq_pop(vm, -1);

        if(assertBool != intendedResult){
            //Send out an event and fail.
            TestingEventBooleanAssertFailed event;
            event.expected = intendedResult;

            SQStackInfos si;
            sq_stackinfos(vm, 1, &si);
            event.lineNum = si.line;
            event.functionName = si.funcname;
            event.srcFile = si.source;

            event.codeLine = _obtainSourceCodeLine(event.srcFile, event.lineNum);

            EventDispatcher::transmitEvent(EventType::Testing, event);
            return SQ_ERROR;
        }
        return 0;
    }

    SQInteger TestNamespace::_processComparisonAssert(HSQUIRRELVM vm, bool equalsComparison){
        SQInteger e = sq_cmp(vm);

        bool comparisonFailed = equalsComparison ? e != 0 : e == 0;
        if(comparisonFailed){
            //Send out an event and fail.
            TestingEventComparisonAssertFailed event;
            event.equals = equalsComparison;

            //First value
            event.firstType = _getTypeString(sq_gettype(vm, -2));
            //Second value
            event.secondType = _getTypeString(sq_gettype(vm, -1));

            sq_tostring(vm, -1);
            //-3 because the previous string has just been pushed onto the stack.
            sq_tostring(vm, -3);

            const SQChar *firstValue;
            const SQChar *secondValue;
            sq_getstring(vm, -1, &firstValue);
            sq_getstring(vm, -2, &secondValue);

            event.firstValue = Ogre::String(firstValue);
            event.secondValue = Ogre::String(secondValue);
            sq_pop(vm, 2);

            SQStackInfos si;
            //Note: The 1 here represents the very bottom of the call stack. If the failing function was called by a method and I put 2 here it would show me the info of that.
            sq_stackinfos(vm, 1, &si);
            event.lineNum = si.line;
            event.functionName = si.funcname;
            event.srcFile = si.source;

            event.codeLine = _obtainSourceCodeLine(event.srcFile, event.lineNum);

            EventDispatcher::transmitEvent(EventType::Testing, event);
            return SQ_ERROR;
        }
        return 0;
    }

    SQInteger TestNamespace::testModeDisabledMessage(HSQUIRRELVM vm){
        AV_WARN("You called a test mode squirrel function when test mode isn't enabled! "
            "To call these functions you need to enable test mode in the avSetup.cfg file.");

        return 0;
    }

    std::string TestNamespace::_obtainSourceCodeLine(const std::string& path, int lineNumber){
        int lineCount = 1;
        std::string line;
        std::ifstream file (path);
        if (file.is_open()){
            while (!file.eof()){
                getline(file,line);
                if(lineCount == lineNumber) return line;
                lineCount++;
            }
            file.close();
        }

        return "";
    }

    std::string TestNamespace::_getTypeString(SQObjectType type){
        std::string retString = "Unknown Type";

        if(type == OT_NULL)
            retString = "null";
        else if(type == OT_INTEGER)
            retString = "integer";
        else if(type == OT_FLOAT)
            retString = "float";
        else if(type == OT_BOOL)
            retString = "bool";
        else if(type == OT_STRING)
            retString = "string";
        else if(type == OT_TABLE)
            retString = "table";
        else if(type == OT_ARRAY)
            retString = "array";
        else if(type == OT_USERDATA)
            retString = "userdata";
        else if(type == OT_CLOSURE)
            retString = "closure(function)";
        else if(type == OT_NATIVECLOSURE)
            retString = "native closure(C function)";
        else if(type == OT_GENERATOR)
            retString = "generator";
        else if(type == OT_USERPOINTER)
            retString = "userpointer";
        else if(type == OT_CLASS)
            retString = "class";
        else if(type == OT_INSTANCE)
            retString = "instance";
        else if(type == OT_WEAKREF)
            retString = "weak reference";

        return retString;
    }

    void TestNamespace::setupNamespace(HSQUIRRELVM vm){
        bool testModeEnabled = SystemSettings::isTestModeEnabled();

        ScriptUtils::RedirectFunctionMap functionMap;
        functionMap["assertTrue"] = {".b", 2, assertTrue};
        functionMap["assertFalse"] = {".b", 2, assertFalse};
        functionMap["assertEqual"] = {"...", 3, assertEqual};
        functionMap["assertNotEqual"] = {"...", 3, assertNotEqual};
        functionMap["endTest"] = {".|.b", -1, endTest};

        ScriptUtils::redirectFunctionMap(vm, testModeDisabledMessage, functionMap, testModeEnabled);

        typedef void(*TestingFunction)(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled);
        typedef std::pair<TestingFunction, const char*> NamespaceEntry;

        const std::vector<NamespaceEntry> e = {
            {TestModeTextureNamespace::setupTestNamespace, "texture"},
            {TestModeSlotManagerNamespace::setupTestNamespace, "slotManager"},
            {TestModeSerialisationNamespace::setupTestNamespace, "serialisation"},
            {TestModePhysicsNamespace::setupTestNamespace, "physics"},
            {TestModeEntityManagerNamespace::setupTestNamespace, "entityManager"},
            {TestModeInputNamespace::setupTestNamespace, "input"},
            {TestModeGuiNamespace::setupTestNamespace, "gui"},
            {TestModeUserComponentNamespace::setupTestNamespace, "userComponents"}
        };

        for(const NamespaceEntry& entry : e){
            sq_pushstring(vm, _SC(entry.second), -1);
            sq_newtable(vm);

            entry.first(vm, testModeDisabledMessage, testModeEnabled);

            sq_newslot(vm, -3 , false);
        }
    }
}

#endif
