#pragma once

#include <string>

namespace AV{
    class SystemSetup;
    
    class SystemSettings{
        friend SystemSetup;
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
    
    public:
        static const std::string& getResourcePath() { return _dataPath; };
        static const std::string& getMasterPath() { return _masterPath; };
        
        static const std::string& getWindowTitleSetting() { return _windowTitle; };
    };
}
