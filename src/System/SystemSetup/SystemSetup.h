#pragma once

#include "OgreString.h"
#include "SystemSettings.h"

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
        static void setup(int argc, char **argv);

    protected:
        static std::string _determineAvSetupPath(int argc, char **argv);

        /**
         Process the avSetup file.
         This is the master config file which resides in the master directory.
         It will contain details about how the engine should be setup.
         */
        static void _processAVSetupFile(Ogre::ConfigFile &file);
        /**
         Process an individual entry in the avSetup file.

         @arg key
         The key from the settings file
         @arg
         The value returned from that key.
         */
        static void _processSettingsFileEntry(const Ogre::String &key, const Ogre::String &value);
        /**
         Process the data directory to find its various files.
         */
        static void _processDataDirectory();
        /**
         Search the data directory to determine if the ogre resources file exists and is viable.

         @param filePath
         The file path to search for the file.
         */
        static void _findOgreResourcesFile(const std::string &filePath);
        /**
         Search the data directory to determine if the squirrel entry file exists and is viable.

         @param filePath
         The file path to search for the file.
         */
        static void _findSquirrelEntryFile(const std::string &filePath);

        /**
         Determine if the maps directory provided exists.

         @param mapsDirectory
         The absolute path of the directory to check.
         */
        static void _findMapsDirectory(const std::string &mapsDirectory);

        static void _determineAvSetupFile(int argc, char **argv);

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
    };
}
