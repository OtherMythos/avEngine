#pragma once

#include <string>
#include <vector>
#include <OgreColourValue.h>
#include "System/EnginePrerequisites.h"
#include "Window/WindowPrerequisites.h"
#include "World/Entity/UserComponents/UserComponentData.h"

namespace AV{
    class SystemSetup;
    class TestModeSerialisationNamespace;

    /**
    A static class to store setting values for the engine.
    This class is populated as part of the SystemSetup class.
    */
    class SystemSettings{
        friend SystemSetup;
        friend TestModeSerialisationNamespace;

    public:
        enum class RenderSystemTypes{
            RENDER_SYSTEM_UNSET,
            RENDER_SYSTEM_OPENGL,
            RENDER_SYSTEM_METAL,
            RENDER_SYSTEM_D3D11,
            RENDER_SYSTEM_VULKAN,
        };
        enum class PlatformTypes{
            PLATFORM_UNKNOWN,
            PLATFORM_WINDOWS,
            PLATFORM_MACOS,
            PLATFORM_LINUX,
            PLATFORM_FREEBSD,
            PLATFORM_IOS,
        };

        struct FontSettingEntry{
            std::string locale;
            std::string fontPath;
            std::string scriptType;
        };


        struct OgreResourceEntry{
            unsigned char groupId;
            std::string path;
        };

        struct PluginEntry{
            //Name to identify the plugin.
            std::string name;
            //Resolved ResPath to the plugin .so file.
            std::vector<std::string> paths;
        };

        typedef std::vector<RenderSystemTypes> RenderSystemContainer;
    private:
        /**
         The path to the data files. These are the static data files that make up the game.
         */
        static std::string _dataPath;
        /**
         The master path is the path in which the engine first searches for files.

         @remarks
         On Linux and Windows it will be the pwd.
         On MacOS it will be the app bundle Resources directory, if the engine is packaged in a bundle.
         */
        static std::string _masterPath;
        /**
         The title of the window.
         Defaults to 'AV Engine'
         */
        static std::string _windowTitle;

        static bool _userSettingsFileViable;
        static std::string mUserSettingsFilePath;

        static std::string _userDirectoryPath;

        /**
         Whether or not the ogre resources file in the data directory is able to be used. i.e, it exists.
         The engine will by default search for a file called OgreResources.cfg in the data directory.
         */
        static bool _ogreResourcesFileViable;

        static bool _squirrelEntryScriptViable;

        /**
         The path to the ogre resources file. This will be relative to the path of the data directory.
         */
        static std::string _ogreResourcesFilePath;

        /**
         The colour of the compositor.
         */
        static Ogre::ColourValue _compositorColour;

        /**
         The path to the squirrel entry file. This will be relative to the path of the data directory.
         */
        static std::string _squirrelEntryScriptPath;

        static std::string mDialogImplementationScript;
        static bool mDialogImplementationScriptViable;

        /**
         The path to the directory containing the maps files. This will be relative to the path of the data directory.
         */
        static std::string mMapsDirectory;

        /**
         The size of a slot in the world. A chunk will be the same size as well.
         */
        static int _worldSlotSize;

        static bool mMapsDirectoryViable;

        static std::string _avSetupFilePath;
        static bool _avSetupFileViable;

#ifdef TEST_MODE
        /**
        Whether or not test mode is enabled. Test mode will alter some aspects of the engine to make it suitable for testing.
        */
        static bool mTestModeEnabled;

        /**
        The name of the test being run by the engine.
        */
        static std::string mTestName;

        static int mTestModeTimeout;
        static bool mTimeoutMeansFail;
#endif

        /**
         The current render system in use.
         */
        static RenderSystemTypes mCurrentRenderSystem;

        /**
         A data structure containing all the available render systems on this platform.
         The first entry is considered the default.
         */
        static RenderSystemContainer mAvailableRenderSystems;

        static std::string mSaveDirectory;
        static bool mSaveDirectoryViable;

        //Whether or not the window is allowed to resize.
        static bool mWindowResizable;

        static Ogre::uint32 mDefaultWindowWidth;
        static Ogre::uint32 mDefaultWindowHeight;
        static FullscreenMode mDefaultFullscreenMode;

        //If true, the engine should create the default action set.
        //It can be disabled in the setup file to improve startup speed.
        static bool mUseDefaultActionSet;

        //If true, setup the default compositor.
        //Not setting it up can be useful if you want to use your own.
        static bool mUseDefaultCompositor;

        //If true, the engine will create the default lights during setup.
        //This is useful for simple projects which don't need any interaction with lights.
        static bool mUseDefaultLights;

        static std::string mProjectName;

        /**
        Write values into the user settings.
        */
        static void _writeIntToUserSettings(const std::string& key, int val);
        static void _writeFloatToUserSettings(const std::string& key, float val);
        static void _writeBoolToUserSettings(const std::string& key, bool val);
        static void _writeStringToUserSettings(const std::string& key, const std::string& val);

        static std::vector<FontSettingEntry> mFontSettings;
        static std::vector<std::string> mSkinPaths;

        static std::vector<std::string> mResourceGroupNames;
        static std::vector<OgreResourceEntry> mResourceEntries;
        static std::vector<PluginEntry> mPluginEntries;

        //How many collision worlds the user actually wants to create.
        static int mNumberCollisionWorlds;

        static bool mDynamicPhysicsDisabled;
        static bool mPhysicsCompletedDisabled;

        static uint8 mNumWorkerThreads;
        static bool mUseSetupFunction;

        static UserComponentSettings mUserComponentSettings;

    public:
        static const std::string& getDataPath() { return _dataPath; };
        static const std::string& getMasterPath() { return _masterPath; };
        static const std::string& getAvSetupFilePath() { return _avSetupFilePath; };
        static const std::string& getUserDirectoryPath() { return _userDirectoryPath; };

        static const std::string& getWindowTitleSetting() { return _windowTitle; };

        static const std::string& getProjectName() { return mProjectName; }

        static const Ogre::ColourValue& getCompositorColourValue() { return _compositorColour; };

        static bool isOgreResourcesFileViable(){ return _ogreResourcesFileViable; };
        /**
         Returns the absolute path to the OgreResourcesFile.
         */
        static const std::string getOgreResourceFilePath(){
            return _ogreResourcesFilePath;
        }

        static bool isSquirrelEntryScriptViable(){ return _squirrelEntryScriptViable; };
        /**
         Returns the absolute path to the squirrelEntryFile.
         */
        static const std::string& getSquirrelEntryScriptPath(){
            return _squirrelEntryScriptPath;
        }

        static bool isMapsDirectoryViable() { return mMapsDirectoryViable; };
        /**
         Return the absolute path to the maps directory.
         */
        static const std::string& getMapsDirectory(){
            return mMapsDirectory;
        }

        static const std::string& getUserSettingsFilePath(){
            return mUserSettingsFilePath;
        }

        static const std::string& getDialogImplementationScriptPath(){
            return mDialogImplementationScript;
        }
        static bool isDialogImplementationScriptViable() { return mDialogImplementationScriptViable; }

        static bool isUserSettingsFileViable() { return _userSettingsFileViable; };

        static bool isAvSetupFileViable() { return _avSetupFileViable; };

        static int getWorldSlotSize(){ return _worldSlotSize; };

#ifdef TEST_MODE
        static bool isTestModeEnabled() { return mTestModeEnabled; };

        static const std::string getTestName() { return mTestName; };

        /**
         Whether or not the timeout being reached in a test means test failure.
         For instance, in a stress test, reaching the timeout might just be a sign that the test passed.
         */
        static bool doesTimeoutMeanFail() { return mTimeoutMeansFail; }

        /**
         Get the number of seconds a test should be run until it times out.
         The default is 120 (2 minutes).
         If set the 0 or less, it will be assumed that no timeout is requested, however this is not advised.
         Regardless of test complexity, some sort of timeout should be specified to mitigate risk.
         */
        static int getTestModeTimeout() { return mTestModeTimeout; };
#endif

        static RenderSystemTypes getCurrentRenderSystem() { return mCurrentRenderSystem; };

        static const RenderSystemContainer& getAvailableRenderSystems() { return mAvailableRenderSystems; };

        static bool isSaveDirectoryViable() { return mSaveDirectoryViable; }

        static bool getUseDefaultActionSet() { return mUseDefaultActionSet; }
        static bool getUseDefaultCompositor() { return mUseDefaultCompositor; }
        static bool getUseDefaultLights() { return mUseDefaultLights; }

        static const std::string& getSaveDirectory(){
            return mSaveDirectory;
        }

        static bool isWindowResizable() { return mWindowResizable; }
        static Ogre::uint32 getDefaultWindowWidth() { return mDefaultWindowWidth; }
        static Ogre::uint32 getDefaultWindowHeight() { return mDefaultWindowHeight; }
        static FullscreenMode getDefaultFullscreenMode() { return mDefaultFullscreenMode; }

        enum class UserSettingType{
            String,
            Int,
            Float,
            Bool
        };
        static bool getUserSetting(const std::string& name, void** outVal, UserSettingType* outType);

        static const std::vector<FontSettingEntry>& getFontSettings() { return mFontSettings; };

        static const std::vector<std::string>& getGuiSkins() { return mSkinPaths; }

        static const std::vector<std::string>& getResourceGroupNames() { return mResourceGroupNames; };
        static const std::vector<OgreResourceEntry>& getResourceEntries() { return mResourceEntries; };

        static const std::vector<PluginEntry>& getPluginEntries() { return mPluginEntries; };

        static int getNumCollisionWorlds() { return mNumberCollisionWorlds; }

        static bool getDynamicPhysicsDisabled() { return mDynamicPhysicsDisabled; }
        static bool getPhysicsCompletelyDisabled() { return mPhysicsCompletedDisabled; }

        static uint8 getNumWorkerThreads() { return mNumWorkerThreads; }

        static bool getUseSetupFunction() { return mUseSetupFunction; }

        static const UserComponentSettings& getUserComponentSettings() { return mUserComponentSettings; }

        static const std::vector<std::string>* getHlmsUserLibrary(const std::string& libName);
        static bool writeHlmsUserLibraryEntry(const std::string& libName, const std::string& path);

    public:

        static void setDefaultWidth(Ogre::uint32 width) { mDefaultWindowWidth = width; }
        static void setDefaultHeight(Ogre::uint32 height) { mDefaultWindowHeight = height; }

        static void setDefaultFullscreenMode(FullscreenMode mode) { mDefaultFullscreenMode = mode; }
    };
}
