#pragma once

#include <OgreString.h>

namespace AV{
    class UserSettingsSetup;

    /**
    A class to store the currently active user settings.
    User settings are values which the engine saves during its setup, for instance which render system does the user want.
    The userSettings file can be used to override the default options for the target platform.
    */
    class UserSettings{
        friend UserSettingsSetup;
    private:
        static float mGuiScale;

        static Ogre::String mRequestedRenderSystem;

        static bool mDeveloperGuiEnabled;

    public:
        static float getGUIScale(){
            return mGuiScale;
        }

        static const Ogre::String& getRequestedRenderSystem() {
            return mRequestedRenderSystem;
        }

        static bool getDeveloperModeGuiEnabled(){
            return mDeveloperGuiEnabled;
        }

        //TODO I might want to hook this into the new fps counter view.
        static void setDeveloperModeGuiEnabled(bool enabled){
            mDeveloperGuiEnabled = enabled;
        }
    };
}
