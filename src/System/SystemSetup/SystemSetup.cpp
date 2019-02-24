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
#include "filesystem/path.h"

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
            AV_INFO("avSetup.cfg file found.");

            SystemSettings::_avSetupFileViable = true;

            SystemSettings::_avSetupFilePath = filesystem::path(avFilePath).parent_path().str();

            _processAVSetupFile(file);
        }
        catch (Ogre::Exception& e)
        {
            if (e.getNumber() == Ogre::Exception::ERR_FILE_NOT_FOUND)
            {
                AV_WARN("No avSetup.cfg file was found at the path {}. Settings will be assumed.", avFilePath);
            }
            else throw;
        }
    }

    std::string SystemSetup::_determineAvSetupPath(int argc, char **argv){
        if(argc > 1){
            std::string argPath = argv[1];
			filesystem::path argPathFile(argPath);

			if(argPathFile.filename() == "avSetup.cfg"){
				if(argPathFile.exists() && argPathFile.is_file()) return argPath;
				else{
					AV_WARN("No valid avSetup.cfg file could be found at the path: {}", argPathFile.str())
				}
			}else{
                AV_WARN("The provided avSetup path should end with avSetup.cfg ! The setup file will be assumed to reside within the master path.")
            }
        }
        //Default value if the provided path was broken, or just not provided.
		filesystem::path retPath = filesystem::path(SystemSettings::getMasterPath()) / filesystem::path("avSetup.cfg");
		return retPath.str();
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
			filesystem::path dataDirectoryPath(value);
            if(dataDirectoryPath.is_absolute()){
				//If the user is providing an absolute path then just go with that.
				if(dataDirectoryPath.exists()) SystemSettings::_dataPath = value;
				else AV_WARN("The data directory path provided ({}) in the avSetup.cfg file is not valid.", value);
            }else{
				//The path is relative
				//Find it as an absolute path for later.
				SystemSettings::_dataPath = (filesystem::path(SystemSettings::getAvSetupFilePath()) / filesystem::path(value)).make_absolute().str();
			}
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
		else if(key == "OgreResourcesFile"){
			SystemSettings::_ogreResourcesFilePath = value;
		}
		else if(key == "MapsDirectory"){
			SystemSettings::mMapsDirectory = value;
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
		//These should be processed later because there's no guarantee that the data directory will have been filled out by that point.
		//When done like this, if a data directory was supplied it will be ready by the time these should be processed.
        _findOgreResourcesFile(SystemSettings::_ogreResourcesFilePath);
        _findSquirrelEntryFile(SystemSettings::_squirrelEntryScriptPath);
        _findMapsDirectory(SystemSettings::mMapsDirectory);

		AV_INFO("OgreResourcesFile set to {}", SystemSettings::getOgreResourceFilePath());
		AV_INFO("SquirrelEntryFile set to {}", SystemSettings::getSquirrelEntryScriptPath());
		AV_INFO("Maps Directory set to {}", SystemSettings::getMapsDirectory());
    }

    void SystemSetup::_findOgreResourcesFile(const std::string &filePath){
		SystemSettings::_ogreResourcesFileViable = false;

		filesystem::path ogrePath(filePath);
		if(!ogrePath.is_absolute()){
			ogrePath = (filesystem::path(SystemSettings::getDataPath()) / ogrePath);

			if(ogrePath.exists()) ogrePath = ogrePath.make_absolute();
		}

		if(ogrePath.exists() && ogrePath.is_file()){
			SystemSettings::_ogreResourcesFilePath = ogrePath.str();
			SystemSettings::_ogreResourcesFileViable = true;
		}
		else AV_WARN("No OgreResources file was found at path {}! No resource locations have been registered with Ogre. This will most likely lead to FileNotFoundExceptions.", ogrePath.str());
    }

    void SystemSetup::_findSquirrelEntryFile(const std::string &filePath){
		SystemSettings::_squirrelEntryScriptViable = false;

		filesystem::path sqPath(filePath);
		if(!sqPath.is_absolute()){
			sqPath = (filesystem::path(SystemSettings::getDataPath()) / sqPath);
			if(sqPath.exists()) sqPath = sqPath.make_absolute();
		}

		if(sqPath.exists() && sqPath.is_file()){
			SystemSettings::_squirrelEntryScriptPath = sqPath.str();
			SystemSettings::_squirrelEntryScriptViable = true;
		}
		else AV_WARN("The Squirrel entry file provided ({}) in the avSetup.cfg file is not valid.", sqPath.str());
    }

    void SystemSetup::_findMapsDirectory(const std::string &mapsDirectory){
		SystemSettings::mMapsDirectoryViable = false;

		filesystem::path mapsPath(mapsDirectory);
		if(!mapsPath.is_absolute()){
			mapsPath = (filesystem::path(SystemSettings::getDataPath()) / mapsPath);
			if(mapsPath.exists()) mapsPath = mapsPath.make_absolute();
		}

		if(mapsPath.exists() && mapsPath.is_directory()){
			SystemSettings::mMapsDirectory = mapsPath.str();
			SystemSettings::mMapsDirectoryViable = true;
		}
		else AV_WARN("The maps directory provided at ({}) in the avSetup.cfg file is not valid.", mapsPath.str());
    }
}
