#include "SystemSetup.h"

#include "Window/SDL2Window/MacOS/MacOSUtils.h"
#include "SystemSettings.h"
#include "System/SystemSetup/SystemSettings.h"

#include <SDL.h>
#include <OgreConfigFile.h>
#include <OgreFileSystemLayer.h>
#include "Logger/Log.h"

#include <OgreStringConverter.h>

namespace AV {
    void SystemSetup::setup(){
        //Start by finding the master path.
        //This will be the pwd on most platforms, and the resources in the app bundle on mac.
        //I'm using SDL to find the bundle path or pwd.
        char *base_path = SDL_GetBasePath();
        SystemSettings::_masterPath = std::string(base_path);
        SDL_free(base_path);
        
        //Now we know the master path check if there's a config file there.
        Ogre::ConfigFile file;
        try {
            file.load(SystemSettings::getMasterPath() + "/avSetup.cfg");
            AV_INFO("avSetup.cfg file found.");
            
            _processAVSetupFile(file);
        }
        catch (Ogre::Exception& e)
        {
            if (e.getNumber() == Ogre::Exception::ERR_FILE_NOT_FOUND)
            {
                AV_WARN("No avSetup.cfg file was found in the master directory. Settings will be assumed.");
            }
            else
            {
                throw;
            }
        }
        
        _processDataDirectory();
    }
    
    void SystemSetup::_processAVSetupFile(Ogre::ConfigFile &file){
        Ogre::ConfigFile::SettingsIterator iter = file.getSettingsIterator();
        while(iter.hasMoreElements())
        {
            Ogre::String archType = iter.peekNextKey();

            Ogre::String filename = iter.getNext();
            
            AV_INFO(archType);
            AV_INFO(filename);
            _processSettingsFileEntry(archType, filename);
        }
    }
    
    void SystemSetup::_processSettingsFileEntry(const Ogre::String &key, const Ogre::String &value){
        if(key == "WindowTitle") SystemSettings::_windowTitle = value;
        if(key == "DataDirectory") SystemSettings::_dataPath = value;
        if(key == "compositorBackground"){
            SystemSettings::_compositorColour = Ogre::StringConverter::parseColourValue(value);
        }
    }
    
    void SystemSetup::_processDataDirectory(){
        _findOgreResourcesFile();
    }
    
    void SystemSetup::_findOgreResourcesFile(){
        Ogre::FileSystemLayer fs("");
        bool fileExists = fs.fileExists(SystemSettings::getResourcePath() + "/OgreResources.cfg");
        
        if(fileExists)
            SystemSettings::_ogreResourcesFileViable = true;
    }
}
