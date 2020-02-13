#include "SystemSetup.h"

#ifdef __APPLE__
	#include "Window/SDL2Window/MacOS/MacOSUtils.h"
#endif
#include "SystemSettings.h"
#include "System/SystemSetup/SystemSettings.h"
#include "UserSettingsSetup.h"
#include "UserSettings.h"

#include <SDL.h>
#include <OgreConfigFile.h>
#include <OgreFileSystemLayer.h>
#include "Logger/Log.h"
#include "filesystem/path.h"

#include <OgreStringConverter.h>

namespace AV {
    void SystemSetup::setup(const std::vector<std::string>& args){
        //Start by finding the master path.
        //This will be the pwd on most platforms, and the resources in the app bundle on mac.
        //I'm using SDL to find the bundle path or pwd.
        char *base_path = SDL_GetBasePath();
        SystemSettings::_masterPath = std::string(base_path);
        SDL_free(base_path);

        _determineAvSetupFile(args);
        _determineUserSettingsFile();

        AV_INFO("Data path set to: " + SystemSettings::getDataPath());

        _processDataDirectory();
        UserSettingsSetup::processUserSettingsFile();

        _determineAvailableRenderSystems();
        SystemSettings::mCurrentRenderSystem = _determineRenderSystem();

#ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()){
            AV_INFO("Test " + SystemSettings::getTestName() + " running.");
        }
#endif
    }

    void SystemSetup::_determineAvSetupFile(const std::vector<std::string>& args){
        //Now we know the master path try and find the setup file.
        Ogre::ConfigFile file;
        std::string avFilePath = _determineAvSetupPath(args);
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

    void SystemSetup::_determineAvailableRenderSystems(){
        #ifdef __APPLE__
            SystemSettings::mAvailableRenderSystems = {
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL,
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL
            };
        #elif __linux__
            SystemSettings::mAvailableRenderSystems = {
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL
            };
        #elif _WIN32
            SystemSettings::mAvailableRenderSystems = {
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11,
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL
            };
        #endif
    }

    SystemSettings::RenderSystemTypes SystemSetup::_determineRenderSystem(){
        SystemSettings::RenderSystemTypes requestedType = _parseRenderSystemString(UserSettings::getRequestedRenderSystem());
        const SystemSettings::RenderSystemContainer& available = SystemSettings::getAvailableRenderSystems();

        if(available.size() <= 0){
            //Call _determineAvailableRenderSystems() first.
            AV_ERROR("No available render systems have been registered.");
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET;
        }

        //See if the requested value is within the available render systems.
        if(std::find(available.begin(), available.end(), requestedType) != available.end()){
            //If it is use that.
            return requestedType;
        }

        //The first value in the vector is considered the default for the platform.
        return available[0];
    }

    SystemSettings::RenderSystemTypes SystemSetup::_parseRenderSystemString(const Ogre::String &rs){
        if(rs == "")
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET;

        if(rs == "Metal"){
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL;
        }else if(rs == "OpenGL"){
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL;
        }else if(rs == "Direct3D11"){
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11;
        }

        return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET;
    }

    void SystemSetup::_determineUserSettingsFile(){
        filesystem::path userSettingsFile = filesystem::path(SystemSettings::getMasterPath()) / filesystem::path("avUserSettings.cfg");
        auto thing = userSettingsFile.str();
        if(userSettingsFile.exists()){
            AV_INFO("User settings file found at path {}", userSettingsFile.str());

            SystemSettings::_userSettingsFileViable = true;
            SystemSettings::mUserSettingsFilePath = userSettingsFile.str();
        }else{
            AV_INFO("No user settings file was found in the master directory.")
        }
    }

    std::string SystemSetup::_determineAvSetupPath(const std::vector<std::string>& args){
        if(args.size() > 1){
            const std::string &argPath = args[1];
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
                filesystem::path p = filesystem::path(SystemSettings::getAvSetupFilePath()) / filesystem::path(value);
                if(p.exists()){
                    SystemSettings::_dataPath = p.make_absolute().str();
                    //Append a directory delimiter to the end of the path.
                    const char* outChar =
                    p.native_path == filesystem::path::path_type::posix_path ? "/" : "\\";
                    SystemSettings::_dataPath.append(outChar);
                }else{
                    AV_WARN("The data directory path provided ({}) in the avSetup.cfg file is not valid.", value);
                }
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
        else if(key == "SaveDirectory"){
            SystemSettings::mSaveDirectory = value;
        }
        else if(key == "WorldSlotSize"){
            SystemSettings::_worldSlotSize = Ogre::StringConverter::parseInt(value);
        }
#ifdef TEST_MODE
        else if(key == "TestMode"){
            SystemSettings::mTestModeEnabled = Ogre::StringConverter::parseBool(value);
        }
        else if(key == "TestName"){
            SystemSettings::mTestName = value;
        }
        else if(key == "TestTimeout"){
            SystemSettings::mTestModeTimeout = Ogre::StringConverter::parseInt(value);
        }
        else if(key == "TestTimeoutMeansFailure"){
            SystemSettings::mTimeoutMeansFail = Ogre::StringConverter::parseBool(value);
        }
#endif
        else if(key == "WindowResizable"){
            SystemSettings::mWindowResizable = Ogre::StringConverter::parseBool(value);
        }
        else if(key == "WindowWidth"){
            _processWindowSize(SystemSettings::mDefaultWindowWidth, Ogre::StringConverter::parseInt(value));
        }
        else if(key == "WindowHeight"){
            _processWindowSize(SystemSettings::mDefaultWindowHeight, Ogre::StringConverter::parseInt(value));
        }
        else if(key == "DialogScript"){
            SystemSettings::mDialogImplementationScript = value;
        }
    }

    void SystemSetup::_processWindowSize(Ogre::uint32& targetVal, int parsedVal){
        Ogre::uint32 t = targetVal;
        if(parsedVal > 0){
            targetVal = static_cast<Ogre::uint32>(parsedVal);
        }
    }

    void SystemSetup::_processDataDirectory(){
        //These should be processed later because there's no guarantee that the data directory will have been filled out by that point.
        //When done like this, if a data directory was supplied it will be ready by the time these should be processed.
        _findOgreResourcesFile();
        _findSquirrelEntryFile();
        _findDialogImplementationFile();
        _findMapsDirectory(SystemSettings::mMapsDirectory);
        _findSaveDirectory(SystemSettings::mSaveDirectory);

        AV_INFO("OgreResourcesFile set to {}", SystemSettings::getOgreResourceFilePath());
        AV_INFO("SquirrelEntryFile set to {}", SystemSettings::getSquirrelEntryScriptPath());
        AV_INFO("Maps Directory set to {}", SystemSettings::getMapsDirectory());
        AV_INFO("Save Directory set to {}", SystemSettings::getSaveDirectory());
    }

    void SystemSetup::_findOgreResourcesFile(){
        if(!_findFile(SystemSettings::_ogreResourcesFileViable, SystemSettings::_ogreResourcesFilePath)){
            AV_WARN("No OgreResources file was found at path {}! No resource locations have been registered with Ogre. This will most likely lead to FileNotFoundExceptions.", SystemSettings::_ogreResourcesFilePath);
        }
    }

    void SystemSetup::_findSquirrelEntryFile(){
        if(!_findFile(SystemSettings::_squirrelEntryScriptViable, SystemSettings::_squirrelEntryScriptPath)){
            AV_WARN("The Squirrel entry file provided ({}) in the avSetup.cfg file is not valid.", SystemSettings::_squirrelEntryScriptPath);
        }
    }

    void SystemSetup::_findDialogImplementationFile(){
        if(!_findFile(SystemSettings::mDialogImplementationScriptViable, SystemSettings::mDialogImplementationScript)){
            AV_WARN("The Dialog implementation file provided at ({}) in the avSetup.cfg file is not valid.", SystemSettings::mDialogImplementationScript);
        }
    }

    bool SystemSetup::_findFile(bool &outViable, std::string& outPath){
        outViable = false;

        filesystem::path fPath(outPath);
        if(!fPath.is_absolute()){
            fPath = (filesystem::path(SystemSettings::getDataPath()) / fPath);
            if(fPath.exists()) fPath = fPath.make_absolute();
        }

        if(fPath.exists() && fPath.is_file()){
            outPath = fPath.str();
            outViable = true;
        }
        else return false;

        return true;
    }

    bool SystemSetup::_findDirectory(const std::string &directory, bool *directoryViable, std::string* directoryPath){
        //SystemSettings::mMapsDirectoryViable = false;
        *directoryViable = false;

        filesystem::path dirPath(directory);
        if(!dirPath.is_absolute()){
            dirPath = (filesystem::path(SystemSettings::getDataPath()) / dirPath);
            if(dirPath.exists()) dirPath = dirPath.make_absolute();
        }

        *directoryPath = dirPath.str();
        if(dirPath.exists() && dirPath.is_directory()){
            *directoryViable = true;
        }else return false;

        return true;
    }

    void SystemSetup::_findMapsDirectory(const std::string &mapsDirectory){
        if(!_findDirectory(mapsDirectory, &SystemSettings::mMapsDirectoryViable, &SystemSettings::mMapsDirectory))
            AV_WARN("The maps directory provided at ({}) is not valid.", mapsDirectory);
    }

    void SystemSetup::_findSaveDirectory(const std::string &saveDirectory){
        //In future the save directory will be set somewhere other than the data directory by default.
        //Really the data directory is supposed to contain read-only data, so it shouldn't be there.
        //The directory will be set depending on platform.
        //TODO this
        if(!_findDirectory(saveDirectory, &SystemSettings::mSaveDirectoryViable, &SystemSettings::mSaveDirectory))
            AV_WARN("The save directory provided at ({}) is not valid.", saveDirectory);
    }
}
