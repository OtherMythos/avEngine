#include "TestModeManager.h"

#include "System/SystemSetup/SystemSettings.h"
#include "Logger/Log.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/TestingEvent.h"
#include "Event/Events/SystemEvent.h"

#include <fstream>
#include <iostream>
#include "filesystem/path.h"

#include "Scripting/ScriptManager.h"

namespace AV{
    TestModeManager::TestModeManager(){

    }

    TestModeManager::~TestModeManager(){

    }

    void TestModeManager::initialise(){
        EventDispatcher::subscribe(EventType::System, AV_BIND(TestModeManager::systemEventReceiver));
        EventDispatcher::subscribe(EventType::Testing, AV_BIND(TestModeManager::testEventReceiver));

        _createTestFile(SystemSettings::getAvSetupFilePath());
    }

    bool TestModeManager::testEventReceiver(const Event &e){
        const TestingEvent& testEvent = (TestingEvent&)e;
        if(testEvent.eventCategory() == TestingEventCategory::booleanAssertFailed
            || testEvent.eventCategory() == TestingEventCategory::comparisonAssertFailed){

            std::vector<std::string> failureMessage = _getFailureMessage(testEvent);
            _printTestFailureMessage(failureMessage);
            _failTest(failureMessage);
        }
        if(testEvent.eventCategory() == TestingEventCategory::testEnd){
            _printTestSuccessMessage();
            _endTest();
        }
		return true;
    }

    bool TestModeManager::systemEventReceiver(const Event &e){
        const SystemEvent& systemEvent = (SystemEvent&)e;
        //The engine is closing normally, so put that into the file.
        if(systemEvent.eventCategory() == SystemEventCategory::EngineClose){
            _endTest();
        }
		return true;
    }

    void TestModeManager::_printTestSuccessMessage(){
        std::string successTitle = "  Test Mode Pass!  ";
        AV_INFO(std::string(successTitle.size(), '='));
        AV_INFO(successTitle);
        AV_INFO(std::string(successTitle.size(), '='));
    }

    void TestModeManager::_printTestFailureMessage(const std::vector<std::string>& message){
        for(const std::string& s : message){
            AV_ERROR(s);
        }
    }

    std::vector<std::string> TestModeManager::_getFailureMessage(const TestingEvent &e){
        std::vector<std::string> retVector;

        std::string failureTitle = "===TESTING MODE FAILURE===";
        retVector.push_back(failureTitle);
        if(e.eventCategory() == TestingEventCategory::booleanAssertFailed){
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
        if(e.eventCategory() == TestingEventCategory::comparisonAssertFailed){
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
