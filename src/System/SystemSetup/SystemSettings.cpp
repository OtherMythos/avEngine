#include "SystemSettings.h"

namespace AV {
    std::string SystemSettings::_dataPath = ".";
    std::string SystemSettings::_masterPath = ".";

    std::string SystemSettings::_windowTitle = "AV Engine";
    Ogre::ColourValue SystemSettings::_compositorColour = Ogre::ColourValue::Black;

    bool SystemSettings::_ogreResourcesFileViable = false;
    std::string SystemSettings::_ogreResourcesFilePath = "OgreResources.cfg";

    std::string SystemSettings::_squirrelEntryScriptPath = "squirrelEntry.nut";
    bool SystemSettings::_squirrelEntryScriptViable = false;

    std::string SystemSettings::_avSetupFilePath = "avSetup.cfg";
    bool SystemSettings::_avSetupFileViable = false;

    std::string SystemSettings::mMapsDirectory = "maps";
    bool SystemSettings::mMapsDirectoryViable = false;

    std::string SystemSettings::mUserSettingsFilePath = "";
    bool SystemSettings::_userSettingsFileViable = false;

    int SystemSettings::_worldSlotSize = 100;

    bool SystemSettings::mTestModeEnabled = false;

    std::string SystemSettings::mTestName = "Unnamed test";
}
