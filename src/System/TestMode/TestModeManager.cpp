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
        if(testEvent.eventCategory() == TestingEventCategory::booleanAssertFailed){
            _failTest();
            ScriptManager::haltForTest();
        }
    }

    bool TestModeManager::systemEventReceiver(const Event &e){
        const SystemEvent& systemEvent = (SystemEvent&)e;
        //The engine is closing normally, so put that into the file.
        if(systemEvent.eventCategory() == SystemEventCategory::EngineClose){
            _endTest();
        }
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
