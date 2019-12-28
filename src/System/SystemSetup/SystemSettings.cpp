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

    std::string SystemSettings::mSaveDirectory = "save";
    bool SystemSettings::mSaveDirectoryViable = false;

    std::string SystemSettings::mUserSettingsFilePath = "";
    bool SystemSettings::_userSettingsFileViable = false;

    int SystemSettings::_worldSlotSize = 100;

#ifdef TEST_MODE
    bool SystemSettings::mTestModeEnabled = false;
    std::string SystemSettings::mTestName = "Unnamed test";
    int SystemSettings::mTestModeTimeout = 120;
    bool SystemSettings::mTimeoutMeansFail = true;
#endif

    bool SystemSettings::mWindowResizable = true;
    Ogre::uint32 SystemSettings::mDefaultWindowWidth = 1600;
    Ogre::uint32 SystemSettings::mDefaultWindowHeight = 1200;

    std::string SystemSettings::mDialogImplementationScript = "dialogImplementation.nut";
    bool SystemSettings::mDialogImplementationScriptViable = false;

    SystemSettings::RenderSystemTypes SystemSettings::mCurrentRenderSystem = RenderSystemTypes::RENDER_SYSTEM_UNSET;

    SystemSettings::RenderSystemContainer SystemSettings::mAvailableRenderSystems = {};
}
