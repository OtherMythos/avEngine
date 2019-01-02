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
        
        /**
         The size of a slot in the world. A chunk will be the same size as well.
         */
        static int _worldSlotSize;
    
    public:
        static const std::string& getDataPath() { return _dataPath; };
        static const std::string& getMasterPath() { return _masterPath; };
        
        static const std::string& getWindowTitleSetting() { return _windowTitle; };
        
        static const Ogre::ColourValue& getCompositorColourValue() { return _compositorColour; };
        
        static bool isOgreResourcesFileViable(){ return _ogreResourcesFileViable; };
        /**
         Returns the absolute path to the OgreResourcesFile.
         */
        static const std::string getOgreResourceFilePath(){
            return getDataPath() + "/" + _ogreResourcesFilePath;
        }
        
        static bool isSquirrelEntryScriptViable(){ return _squirrelEntryScriptViable; };
        /**
         Returns the absolute path to the squirrelEntryFile.
         */
        static const std::string getSquirrelEntryScriptPath(){
            return getDataPath() + "/" + _squirrelEntryScriptPath;
        }
        
        static int getWorldSlotSize(){ return _worldSlotSize; };
    };
}
