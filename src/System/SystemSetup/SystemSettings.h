#pragma once

#include <string>
#include <vector>
#include <OgreColourValue.h>

namespace AV{
    class SystemSetup;
    class TestModeSerialisationNamespace;

    class SystemSettings{
        friend SystemSetup;
        friend TestModeSerialisationNamespace;

    public:
        enum class RenderSystemTypes{
            RENDER_SYSTEM_UNSET,
            RENDER_SYSTEM_OPENGL,
            RENDER_SYSTEM_METAL,
            RENDER_SYSTEM_D3D11
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

        /**
        Write values into the user settings.
        */
        static void _writeIntToUserSettings(const std::string& key, int val);
        static void _writeFloatToUserSettings(const std::string& key, float val);
        static void _writeBoolToUserSettings(const std::string& key, bool val);
        static void _writeStringToUserSettings(const std::string& key, const std::string& val);

    public:
        static const std::string& getDataPath() { return _dataPath; };
        static const std::string& getMasterPath() { return _masterPath; };
        static const std::string& getAvSetupFilePath() { return _avSetupFilePath; };

        static const std::string& getWindowTitleSetting() { return _windowTitle; };

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

        static const std::string& getSaveDirectory(){
            return mSaveDirectory;
        }

        static bool isWindowResizable() { return mWindowResizable; }
        static Ogre::uint32 getDefaultWindowWidth() { return mDefaultWindowWidth; }
        static Ogre::uint32 getDefaultWindowHeight() { return mDefaultWindowHeight; }
    };
}
