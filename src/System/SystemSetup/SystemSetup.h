#pragma once

#include "OgreString.h"
#include "SystemSettings.h"

#include "rapidjson/document.h"

namespace Ogre {
    class ConfigFile;
}

namespace AV {
    class SystemSetup{
    public:
        /**
         Setup the system settings. This will involve determining the master directory and searching it for all the various setup information.
         This function should be one of the first called in the setup routine, as without these settings determined much of the other parts of the engine can't function as intended.
         */
        static void setup(const std::vector<std::string>& args);

    protected:
        static std::string _determineAvSetupPath(const std::vector<std::string>& args);

        /**
         Process the avSetup file.
         This is the master config file which resides in the master directory.
         It will contain details about how the engine should be setup.

         @param filePath
         A path to a file which contains the setup content as a json.
         This function does not check the validity of the file path.

         @returns
         True or false depending on whether the setup file was processed correctly.
         */
        static bool _processAVSetupFile(const std::string& filePath);
        /**
        Read through a rapid json document and pull the settings from it.
        */
        static bool _processAVSetupDocument(rapidjson::Document& document);
        /**
         Process an individual entry in the avSetup file.

         @arg key
         The key from the settings file
         @arg
         The value returned from that key.
         */
        static void _processSettingsFileEntry(const Ogre::String &key, const Ogre::String &value);
        /**
         Process an entry in the avSetup file from the user settings section.
         Any entry that's in its own group is set as a user setting.

         @arg val
         The user settings json value which is to be passed. This is expected to be a json object.
         */
        static void _processSettingsFileUserEntries(const rapidjson::Value &val);
        static void _processGuiSettings(const rapidjson::Value &val);
        static void _processOgreResources(const rapidjson::Value &val);
        static void _addOgreResourceLocation(const char* groupName, const char* path);
        /**
         Process the data directory to find its various files.
         */
        static void _processDataDirectory();
        /**
         Search the data directory to determine if the ogre resources file exists and is viable.
         */
        static void _findOgreResourcesFile();
        /**
         Search the data directory to determine if the squirrel entry file exists and is viable.
         */
        static void _findSquirrelEntryFile();

        static void _findDialogImplementationFile();

        /**
        Convenience function to find a file.
        */
        static bool _findFile(bool &outViable, std::string& outPath);

        /**
         Determine if the maps directory provided exists.

         @param mapsDirectory
         The absolute path of the directory to check.
         */
        static void _findMapsDirectory(const std::string &mapsDirectory);

        static void _findSaveDirectory(const std::string &saveDirectory);

        static bool _findDirectory(const std::string &directory, bool *directoryViable, std::string* directoryPath);

        static void _determineAvSetupFile(const std::vector<std::string>& args);

        /**
        Find the user settings file.
        The user settings file contains user specific settings for the engine, such as render system settings, resolution, graphical settings, etc.
        If no user settings file is found sensible defaults will be setup in UserSettings.
        */
        static void _determineUserSettingsFile();

        /**
         Determine the render systems which are available on this system.
         This will set the values inside of SystemSettings.
         */
        static void _determineAvailableRenderSystems();

        /**
         Determine the render system that the engine should use.

         If the render system requested by the user in UserSettings is valid it will be set as the target render system.
         A render system is valid when it is present in the available render systems in system setup.

         @remarks
         You need to call _determineAvailableRenderSystems() before this.
         */
        static SystemSettings::RenderSystemTypes _determineRenderSystem();

        /**
         Parse a string and return a render system enum value.

         @param rs
         A string containing a render system name.
         */
        static SystemSettings::RenderSystemTypes _parseRenderSystemString(const Ogre::String &rs);

        //Parse a window size. The resolved size will be written to targetVal, after it's been sanity checked.
        static void _processWindowSize(Ogre::uint32& targetVal, int parsedVal);

        /**
        Process the values specified regarding the collision world.
        */
        static void _parseCollisionWorldSettings(rapidjson::Document& d);

        /**
        Process settings of the dynamics world.
        */
        static void _parseDynamicWorldSettings(const rapidjson::Value& d);

        /**
        Parse settings for the user components.
        */
        static void _parseComponentSettings(const rapidjson::Value& d);
    };
}
