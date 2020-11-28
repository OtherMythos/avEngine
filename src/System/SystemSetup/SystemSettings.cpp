#include "SystemSettings.h"

#include <map>
#include <vector>

namespace AV {
    std::string SystemSettings::_dataPath = ".";
    std::string SystemSettings::_masterPath = ".";

    std::string SystemSettings::_windowTitle = "";
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

    bool SystemSettings::mUseDefaultActionSet = true;

    std::string SystemSettings::mProjectName = "";

    int SystemSettings::mNumberCollisionWorlds = 0;
    bool SystemSettings::mDynamicPhysicsDisabled = false;
    //This is not a user setting. The engine disables physics (and the physics thread) completely if there is no dynamics world or num collision worlds is 0.
    bool SystemSettings::mPhysicsCompletedDisabled = false;

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

    std::vector<SystemSettings::FontSettingEntry> SystemSettings::mFontSettings;
    std::vector<std::string> SystemSettings::mSkinPaths;

    uint8 SystemSettings::mNumWorkerThreads = 2;

    UserComponentSettings SystemSettings::mUserComponentSettings;

    union UserSettingsEntryType{
        float f;
        int i;
        bool b;
        unsigned int s;
    };
    //typedef std::pair<unsigned int, UserSettingType> UserSettingEntry;
    typedef std::pair<UserSettingsEntryType, SystemSettings::UserSettingType> UserSettingEntry;
    //The user settings map is stored in the cpp file rather than the header so I don't have to include map and vector in the header (which is then used in lots of other files).
    //This map stores entries by string as well as an int to refernece them in one of the vectors.
    std::map<std::string, UserSettingEntry> mUserSettings;

    std::vector<std::string> SystemSettings::mResourceGroupNames;
    std::vector<SystemSettings::OgreResourceEntry> SystemSettings::mResourceEntries;

    std::vector<std::string>* mUserStrings = 0;
    void SystemSettings::_writeIntToUserSettings(const std::string& key, int val){
        UserSettingsEntryType v;
        v.i = val;
        mUserSettings[key] = {v, UserSettingType::Int};
    }

    void SystemSettings::_writeFloatToUserSettings(const std::string& key, float val){
        UserSettingsEntryType v;
        v.f = val;
        mUserSettings[key] = {v, UserSettingType::Float};
    }

    void SystemSettings::_writeBoolToUserSettings(const std::string& key, bool val){
        UserSettingsEntryType v;
        v.b = val;
        mUserSettings[key] = {v, UserSettingType::Bool};
    }

    void SystemSettings::_writeStringToUserSettings(const std::string& key, const std::string& val){
        if(!mUserStrings) mUserStrings = new std::vector<std::string>();

        UserSettingsEntryType v;
        v.s = mUserStrings->size();
        mUserStrings->push_back(val);
        mUserSettings[key] = {v, UserSettingType::String};
    }

    bool SystemSettings::getUserSetting(const std::string& name, void** outVal, UserSettingType* outType){
        auto it = mUserSettings.find(name);
        if(it == mUserSettings.end()) return false;

        UserSettingEntry& e = (*it).second;
        *outType = e.second;

        if(e.second == UserSettingType::String){
            assert(mUserStrings);
            std::string& strVal = (*mUserStrings)[e.first.s];
            void* ptr = reinterpret_cast<void*>( &strVal );
            *outVal = ptr;
        }else{
            void* ptr = reinterpret_cast<void*>( &(e.first.i) );
            *outVal = ptr;
        }

        return true;
    }

}
