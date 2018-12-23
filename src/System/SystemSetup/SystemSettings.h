#pragma once

#include <string>
#include <OgreColourValue.h>

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
        
        /**
         Whether or not the ogre resources file in the data directory is able to be used. i.e, it exists.
         */
        static bool _ogreResourcesFileViable;
        
        static Ogre::ColourValue _compositorColour;
    
    public:
        static const std::string& getResourcePath() { return _dataPath; };
        static const std::string& getMasterPath() { return _masterPath; };
        
        static const std::string& getWindowTitleSetting() { return _windowTitle; };
        
        static const Ogre::ColourValue& getCompositorColourValue() { return _compositorColour; };
        
        static bool isOgreResourcesFileViable(){ return _ogreResourcesFileViable; };
    };
}
