#include "TestModeManager.h"

#include "System/SystemSetup/SystemSettings.h"
#include "Logger/Log.h"
#include "Event/EventDispatcher.h"

#include <fstream>
#include <iostream>
#include <OgreFileSystemLayer.h>

namespace AV{
    TestModeManager::TestModeManager(){

    }

    TestModeManager::~TestModeManager(){

    }

    void TestModeManager::initialise(){
        EventDispatcher::subscribe(EventType::System, AV_BIND(TestModeManager::worldEventReceiver));

        _createTestFile(SystemSettings::getAvSetupFilePath());
    }

    bool TestModeManager::worldEventReceiver(const Event &e){
        //The engine is closing normally, so put that into the file.

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
