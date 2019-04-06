#pragma once

#include <OgreString.h>

namespace AV{
    class UserSettingsSetup;

    /**
    A class to store the currently active user settings.
    */
    class UserSettings{
        friend UserSettingsSetup;
    private:
        static float mGuiScale;
        
        static Ogre::String mRequestedRenderSystem;

    public:
        static float getGUIScale(){
            return mGuiScale;
        }
        
        static const Ogre::String& getRequestedRenderSystem() {
            return mRequestedRenderSystem;
        }
    };
}
