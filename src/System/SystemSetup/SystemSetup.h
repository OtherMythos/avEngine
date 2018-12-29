#pragma once

#include "OgreString.h"

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
        static void setup();
        
    protected:
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
    };
}
