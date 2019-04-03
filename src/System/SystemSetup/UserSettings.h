#pragma once

namespace AV{
    class UserSettingsSetup;

    /**
    A class to store the currently active user settings.
    */
    class UserSettings{
        friend UserSettingsSetup;
    private:
        static float mGuiScale;

    public:
        static float getGUIScale(){
            return mGuiScale;
        }
    };
}
