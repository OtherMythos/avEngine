#pragma once

#include <OgreString.h>

namespace Ogre{
    class ConfigFile;
};

namespace AV{
    class UserSettingsSetup{
    public:
        /**
        Process the user settings file as defined in SystemSettings.
        If no path was supplied or the file is invalid nothing will be processed.
        */
        static void processUserSettingsFile();

        /**
        Determine some sensible defaults based on platform .
        This function is run when the engine starts.
        It will determine sensible defaults for things like rendering apis, content scaling, and so on.

        Example:
        MacOS is the platform, we should be using the metal rendering system.
        If it is not available (old hardware), then the default is opengl.
        */
        static void determineSensibleDefaults();

    private:
        static void _processUserSettingsFile(Ogre::ConfigFile &file);

        static void _processEntry(const Ogre::String &key, const Ogre::String &value);
    };
}
