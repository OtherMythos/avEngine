#include "TestModeSerialisationNamespace.h"

#include "filesystem/path.h"
#include "System/SystemSetup/SystemSettings.h"

#include "OgreSceneNode.h"

namespace AV{
    SQInteger TestModeSerialisationNamespace::assureSaveDirectory(HSQUIRRELVM vm){
        //Assure the save directory exists.
        filesystem::create_directory(SystemSettings::getSaveDirectory());

        SystemSettings::mSaveDirectoryViable = true;

        return 0;
    }

    void TestModeSerialisationNamespace::setupTestNamespace(HSQUIRRELVM vm, SQFUNCTION messageFunction, bool testModeEnabled){
        RedirectFunctionMap functionMap;
        functionMap["assureSaveDirectory"] = {"", 0, assureSaveDirectory};

        _redirectFunctionMap(vm, messageFunction, functionMap, testModeEnabled);
    }
}
