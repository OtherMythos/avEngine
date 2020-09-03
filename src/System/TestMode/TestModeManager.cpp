#ifdef TEST_MODE

#include "TestModeManager.h"

#include "System/SystemSetup/SystemSettings.h"
#include "Logger/Log.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/TestingEvent.h"
#include "Event/Events/SystemEvent.h"

#include <fstream>
#include <iostream>
#include "filesystem/path.h"

#include "Scripting/ScriptVM.h"

namespace AV{
    TestModeManager::TestModeManager(){

    }

    TestModeManager::~TestModeManager(){
        //Unsubscribe to stuff on shutdown.
        //Probably doesn't matter anyway because it's only ever called on complete engine shutdown.
        EventDispatcher::unsubscribe(EventType::System, this);
        EventDispatcher::unsubscribe(EventType::Testing, this);
    }

    void TestModeManager::initialise(){
        EventDispatcher::subscribe(EventType::System, AV_BIND(TestModeManager::systemEventReceiver));
        EventDispatcher::subscribe(EventType::Testing, AV_BIND(TestModeManager::testEventReceiver));

        _createTestFile(SystemSettings::getAvSetupFilePath());

        AV_INFO("This test will timeout after {} seconds", SystemSettings::getTestModeTimeout());
        startTime = std::chrono::system_clock::now();
    }

    bool TestModeManager::testEventReceiver(const Event &e){
        const TestingEvent& testEvent = (TestingEvent&)e;
        if(testEvent.eventId() == EventId::TestingBooleanAssertFailed
           || testEvent.eventId() == EventId::TestingComparisonAssertFailed
           || testEvent.eventId() == EventId::TestingScriptFailure){

            //The tests stop execution by throwing an error, which would otherwise be picked up here.
            //If a null event happens then don't print anything out.
            bool correctFailure = true;
            if(testEvent.eventId() == EventId::TestingScriptFailure){
                const TestingEventScriptFailure& b = (TestingEventScriptFailure&)e;
                if(b.failureReason == "(null : 0x(nil))") correctFailure = false;
            }

            if(correctFailure && !testFailed && !testFinished){
                _eventFailTest(testEvent);
            }
        }
        if(testEvent.eventId() == EventId::TestingTestEnd){
            const TestingEventTestEnd& end = (TestingEventTestEnd&)e;
            if(end.successfulEnd){
                _printTestSuccessMessage();
                _endTest();
            }else{
                _eventFailTest(testEvent);
            }
            testFinished = true;
        }
        if(testEvent.eventId() == EventId::TestingTimeoutReached){
            const TestingEventTimeoutReached& end = (TestingEventTimeoutReached&)e;
            if(end.meansFailure){
                _eventFailTest(testEvent);
            }else{
                _printTestSuccessMessage();
                _endTest();
            }
        }
		return true;
    }

    void TestModeManager::updateTimeout(){
        int timeout = SystemSettings::getTestModeTimeout();
        if(timeout <= 0) return;

        auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> diff = now-startTime;
        if(diff.count() >= timeout){
            //The timeout has been reached.
            _processTimeout(SystemSettings::doesTimeoutMeanFail(), timeout);
        }
    }

    void TestModeManager::_processTimeout(bool meansFail, int time){
        TestingEventTimeoutReached e;
        e.meansFailure = meansFail;
        e.totalSeconds = time;
        EventDispatcher::transmitEvent(EventType::Testing, e);
    }

    void TestModeManager::_eventFailTest(const TestingEvent& testEvent){
        std::vector<std::string> failureMessage = _getFailureMessage(testEvent);
        _printTestFailureMessage(failureMessage);
        _failTest(failureMessage);
    }

    bool TestModeManager::systemEventReceiver(const Event &e){
        const SystemEvent& systemEvent = (SystemEvent&)e;
        //The engine is closing normally, so put that into the file.
        if(systemEvent.eventId() == EventId::SystemEngineClose){
            _endTest();
        }
        return true;
    }

    void TestModeManager::_printTestSuccessMessage(){
        std::string successTitle = "  Test Mode Pass!  ";
        std::string successDesc = "  Test Case " + SystemSettings::getTestName() + "  ";

        AV_INFO(std::string(successDesc.size(), '='));

        AV_INFO(successTitle);
        AV_INFO(successDesc);
        AV_INFO(std::string(successDesc.size(), '='));
    }

    void TestModeManager::_printTestFailureMessage(const std::vector<std::string>& message){
        for(const std::string& s : message){
            AV_ERROR(s);
        }
    }

    std::vector<std::string> TestModeManager::_getFailureMessage(const TestingEvent &e){
        std::vector<std::string> retVector;

        std::string failureTitle = "===TESTING MODE FAILURE===";
        retVector.push_back("Test Case " + SystemSettings::getTestName());
        retVector.push_back(failureTitle);
        if(e.eventId() == EventId::TestingBooleanAssertFailed){
            const TestingEventBooleanAssertFailed& b = (TestingEventBooleanAssertFailed&)e;
            std::string expected = b.expected ? "True" : "False";
            std::string received = !b.expected ? "True" : "False";

            retVector.push_back("Assert " + expected + " failed!");
            retVector.push_back("  Expected: " + expected);
            retVector.push_back("  Received: " + received);
            retVector.push_back("");
            retVector.push_back("On line " + std::to_string(b.lineNum) + " in function " + b.functionName);
            retVector.push_back("  " + b.codeLine);
            retVector.push_back("Of source file " + b.srcFile);
        }
        if(e.eventId() == EventId::TestingComparisonAssertFailed){
            const TestingEventComparisonAssertFailed& b = (TestingEventComparisonAssertFailed&)e;
            std::string assertType = b.equals ? "equal" : "not equal";
            retVector.push_back("Assert " + assertType + " failed!");
            retVector.push_back("  Expected: " + b.firstType);
            retVector.push_back("      With value: " + b.firstValue);
            retVector.push_back("  Received: " + b.secondType);
            retVector.push_back("      With value: " + b.secondValue);
            retVector.push_back("");
            retVector.push_back("On line " + std::to_string(b.lineNum) + " in function " + b.functionName);
            retVector.push_back("  " + b.codeLine);
            retVector.push_back("Of source file " + b.srcFile);
        }
        if(e.eventId() == EventId::TestingScriptFailure){
            const TestingEventScriptFailure& b = (TestingEventScriptFailure&)e;
            retVector.push_back("The script " + b.srcFile + " failed during execution.");
            retVector.push_back("   Reason: " + b.failureReason);
            //retVector.push_back("On line " + std::to_string(b.lineNum) + " in function " + b.functionName);
            //retVector.push_back("Of source file " + b.srcFile);
        }
        if(e.eventId() == EventId::TestingTestEnd){
            const TestingEventTestEnd& b = (TestingEventTestEnd&)e;
            retVector.push_back("_test.endTest() was called with a value of false.");
            retVector.push_back("In line " + std::to_string(b.lineNum) + " of script " + b.srcFile);
        }
        if(e.eventId() == EventId::TestingTimeoutReached){
            const TestingEventTimeoutReached& b = (TestingEventTimeoutReached&)e;
            retVector.push_back("The testTimeout was reached after " + std::to_string(b.totalSeconds) + " seconds.");
        }
        retVector.push_back(std::string(failureTitle.size(), '='));

        return retVector;
    }

    void TestModeManager::_failTest(const std::vector<std::string>& failMessage){
        std::ofstream outfile;
        outfile.open(testFilePath.c_str());

        outfile << SystemSettings::getTestName() << std::endl;
        outfile << "-1" << std::endl;
        outfile << std::endl;

        for(const std::string& s : failMessage){
            outfile << s << std::endl;
        }

        outfile.close();

        testFailed = true;

        AV_INFO("Test failure state written to file.");
    }

    void TestModeManager::_endTest(){
        //If the test has already failed don't write success to the file under any circumstances.
        if(testFailed) return;

        std::ofstream outfile;
        outfile.open(testFilePath.c_str());

        outfile << SystemSettings::getTestName() << std::endl;
        outfile << "1" << std::endl;

        outfile.close();

        AV_INFO("Test state written to file.");
    }

    void TestModeManager::_createTestFile(const std::string &path){
        filesystem::path pathHandle = (filesystem::path(path) / filesystem::path("avTestFile.txt"));
        testFilePath = pathHandle.str();

        if(pathHandle.exists() && pathHandle.is_file()){
            remove(testFilePath.c_str());
        }

        std::ofstream outfile;
        outfile.open(testFilePath.c_str());

        outfile << SystemSettings::getTestName() << std::endl;
        //0 means the test is still running.
        outfile << "0" << std::endl;

        outfile.close();
    }
}

#endif
