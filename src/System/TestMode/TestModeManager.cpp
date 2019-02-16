#include "TestModeManager.h"

#include "System/SystemSetup/SystemSettings.h"
#include "Logger/Log.h"
#include "Event/EventDispatcher.h"
#include "Event/Events/TestingEvent.h"
#include "Event/Events/SystemEvent.h"

#include <fstream>
#include <iostream>
#include <OgreFileSystemLayer.h>

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
            _printTestFailureMessage(testEvent);
            _failTest();
        }
    }

    bool TestModeManager::systemEventReceiver(const Event &e){
        const SystemEvent& systemEvent = (SystemEvent&)e;
        //The engine is closing normally, so put that into the file.
        if(systemEvent.eventCategory() == SystemEventCategory::EngineClose){
            _endTest();
        }
    }

    void TestModeManager::_printTestFailureMessage(const TestingEvent &e){
        std::string failureTitle = "===TESTING MODE FAILURE===";
        AV_ERROR(failureTitle);

        if(e.eventCategory() == TestingEventCategory::booleanAssertFailed){
            const TestingEventBooleanAssertFailed& b = (TestingEventBooleanAssertFailed&)e;
            std::string expected = b.expected ? "True" : "False";
            std::string received = !b.expected ? "True" : "False";
            AV_ERROR("Assert " + expected + " failed!");
            AV_ERROR("  Expected: " + expected);
            AV_ERROR("  Received: " + received);
            AV_ERROR("");
            AV_ERROR("On line {} in function {}", b.lineNum, b.functionName);
            AV_ERROR("  " + b.codeLine);
            AV_ERROR("Of source file {}", b.srcFile);
        }
        if(e.eventCategory() == TestingEventCategory::comparisonAssertFailed){
            const TestingEventComparisonAssertFailed& b = (TestingEventComparisonAssertFailed&)e;
            std::string assertType = b.equals ? "equal" : "not equal";
            AV_ERROR("Assert " + assertType + " failed!");
            AV_ERROR("  Expected: " + b.firstType);
            AV_ERROR("      Of type: " + b.firstValue);
            AV_ERROR("  Received: " + b.secondType);
            AV_ERROR("      Of type: " + b.secondValue);
            AV_ERROR("");
            AV_ERROR("On line {} in function {}", b.lineNum, b.functionName);
            AV_ERROR("  " + b.codeLine);
            AV_ERROR("Of source file {}", b.srcFile);
        }
        AV_ERROR(std::string(failureTitle.size(), '='));
    }

    void TestModeManager::_failTest(){
        std::ofstream outfile;
        outfile.open(testFilePath.c_str());

        outfile << SystemSettings::getTestName() << std::endl;
        outfile << "-1" << std::endl;

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
        testFilePath = path + "avTestFile.txt";

        //Delete the old file if it already exists.
        Ogre::FileSystemLayer fs("");
        bool fileExists = fs.fileExists(testFilePath);
        if(fileExists){
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
