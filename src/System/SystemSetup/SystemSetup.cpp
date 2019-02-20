#include "SystemSetup.h"

#ifdef __APPLE__
	#include "Window/SDL2Window/MacOS/MacOSUtils.h"
#endif
#include "SystemSettings.h"
#include "System/SystemSetup/SystemSettings.h"

#include <SDL.h>
#include <OgreConfigFile.h>
#include <OgreFileSystemLayer.h>
#include "Logger/Log.h"

#include <OgreStringConverter.h>

namespace AV {
    void SystemSetup::setup(int argc, char **argv){
        //Start by finding the master path.
        //This will be the pwd on most platforms, and the resources in the app bundle on mac.
        //I'm using SDL to find the bundle path or pwd.
        char *base_path = SDL_GetBasePath();
        SystemSettings::_masterPath = std::string(base_path);
        SDL_free(base_path);

        _determineAvSetupFile(argc, argv);

        AV_INFO("Data path set to: " + SystemSettings::getDataPath());

        _processDataDirectory();
    }

    void SystemSetup::_determineAvSetupFile(int argc, char **argv){
        //Now we know the master path try and find the setup file.
        Ogre::ConfigFile file;
        std::string avFilePath = _determineAvSetupPath(argc, argv);
        try {
            file.load(avFilePath);
			//file.load("C:\\Users\\edward\\Documents\\avEngine\\build\\Debug\\avSetup.cfg");
            AV_INFO("avSetup.cfg file found.");

            SystemSettings::_avSetupFileViable = true;

            std::string avFilePathShaved = avFilePath;
            avFilePathShaved.erase(avFilePathShaved.length() - 11);

            SystemSettings::_avSetupFilePath = avFilePathShaved;

            _processAVSetupFile(file);
        }
        catch (Ogre::Exception& e)
        {
            if (e.getNumber() == Ogre::Exception::ERR_FILE_NOT_FOUND)
            {
                AV_WARN("No avSetup.cfg file was found at the path {}. Settings will be assumed.", avFilePath);
            }
            else
            {
                throw;
            }
        }
    }

    std::string SystemSetup::_determineAvSetupPath(int argc, char **argv){
        if(argc > 1){
            std::string argPath = argv[1];
            if(_ends_with(argPath, "avSetup.cfg")) return argPath;
            else{
                AV_WARN("The provided avSetup path should end with avSetup.cfg ! The setup file will be assumed to reside within the master path.")
            }
        }
        //Default value if the provided path was broken, or just not provided.
#ifdef WIN32
		return SystemSettings::getMasterPath() + "avSetup.cfg";
#else
        return SystemSettings::getMasterPath() + "/avSetup.cfg";
#endif
    }

    void SystemSetup::_processAVSetupFile(Ogre::ConfigFile &file){
        Ogre::ConfigFile::SettingsIterator iter = file.getSettingsIterator();
        while(iter.hasMoreElements())
        {
            Ogre::String archType = iter.peekNextKey();

            Ogre::String filename = iter.getNext();

            _processSettingsFileEntry(archType, filename);
        }
    }

    void SystemSetup::_processSettingsFileEntry(const Ogre::String &key, const Ogre::String &value){
        if(key == "WindowTitle") SystemSettings::_windowTitle = value;
        else if(key == "DataDirectory") {
            //If the user is providing an absolute path then just go with that.
            if(value[0] != "/"[0]){
                SystemSettings::_dataPath = SystemSettings::getAvSetupFilePath() + value;
            }else
                SystemSettings::_dataPath = value;
        }
        else if(key == "CompositorBackground"){
            SystemSettings::_compositorColour = Ogre::StringConverter::parseColourValue(value);
        }
        else if(key == "ResourcesFile"){
            SystemSettings::_ogreResourcesFilePath = value;
        }
        else if(key == "SquirrelEntryFile"){
            SystemSettings::_squirrelEntryScriptPath = value;
        }
        else if(key == "WorldSlotSize"){
            SystemSettings::_worldSlotSize = Ogre::StringConverter::parseInt(value);
        }
        else if(key == "MapsDirectory"){
            SystemSettings::mMapsDirectory = value;
        }
        else if(key == "TestMode"){
            SystemSettings::mTestModeEnabled = Ogre::StringConverter::parseBool(value);
        }
        else if(key == "TestName"){
            SystemSettings::mTestName = value;
        }
    }

    void SystemSetup::_processDataDirectory(){
        _findOgreResourcesFile(SystemSettings::getOgreResourceFilePath());
        _findSquirrelEntryFile(SystemSettings::getSquirrelEntryScriptPath());
        _findMapsDirectory(SystemSettings::getMapsDirectory());
    }

    void SystemSetup::_findOgreResourcesFile(const std::string &filePath){
        Ogre::FileSystemLayer fs("");
        bool fileExists = fs.fileExists(filePath);

        if(fileExists){
            SystemSettings::_ogreResourcesFileViable = true;
        }else{
            AV_WARN("The OgreResources setup file wasn't found! No resource locations have been registered with Ogre. This will most likely lead to FileNotFoundExceptions.");
        }
    }

    void SystemSetup::_findSquirrelEntryFile(const std::string &filePath){
        Ogre::FileSystemLayer fs("");
        bool fileExists = fs.fileExists(filePath);

        if(fileExists){
            SystemSettings::_squirrelEntryScriptViable = true;
        }else{
            AV_WARN("The Squirrel entry script wasn't found! This will likely mean little engine functionality.");
        }
    }

    void SystemSetup::_findMapsDirectory(const std::string &mapsDirectory){
        Ogre::FileSystemLayer fs("");
        bool directoryExists = fs.fileExists(mapsDirectory);

        if(directoryExists){
            SystemSettings::mMapsDirectoryViable = true;
        }else{
            AV_WARN("The maps directory couldn't be found. This will mean issues loading map chunks.");
        }
    }
}
