#include "UserSettingsSetup.h"

#include "SystemSettings.h"
#include "Logger/Log.h"
#include "UserSettings.h"
#include <OgreStringConverter.h>
#include <OgreConfigFile.h>

namespace AV{

    void UserSettingsSetup::determineSensibleDefaults(){
        //TODO determine if this needs to be implemented.
        //I've realised that I have ogre setup for specific platforms anyway.
        //It might be useful for gui scaling or something like that.
    }

    void UserSettingsSetup::processUserSettingsFile(){
        if(!SystemSettings::isUserSettingsFileViable())
            return;

        Ogre::ConfigFile file;
        const std::string& userSettingsPath = SystemSettings::getUserSettingsFilePath();
        try {
            file.load(userSettingsPath);

            _processUserSettingsFile(file);

        }catch (Ogre::Exception& e){
            AV_ERROR("An error occurred while parsing the avUserSettings.cfg file.")
        }
    }

    void UserSettingsSetup::_processUserSettingsFile(Ogre::ConfigFile &file){
        Ogre::ConfigFile::SettingsIterator iter = file.getSettingsIterator();
        while(iter.hasMoreElements()){
            Ogre::String archType = iter.peekNextKey();

            Ogre::String filename = iter.getNext();

            _processEntry(archType, filename);
        }
    }

    void UserSettingsSetup::_processEntry(const Ogre::String &key, const Ogre::String &value){
        if(key == "GuiScale"){
            UserSettings::mGuiScale = Ogre::StringConverter::parseReal(value);
        }else if(key == "RenderSystem"){
            UserSettings::mRequestedRenderSystem = value;
        }
    }
}
