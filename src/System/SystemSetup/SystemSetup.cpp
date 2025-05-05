#include "SystemSetup.h"

#ifdef __APPLE__
    #include "Window/SDL2Window/MacOS/MacOSUtils.h"
#endif
#include "SystemSettings.h"
#include "System/SystemSetup/SystemSettings.h"
#include "System/EnginePrerequisites.h"
#include "UserSettingsSetup.h"
#include "UserSettings.h"
#include "World/Entity/UserComponents/UserComponentData.h"
#include "Dialog/DialogSettings.h"
#include "System/FileSystem/FilePath.h"
#include <filesystem>

#ifdef __APPLE__
    #include "Window/SDL2Window/MacOS/MacOSUtils.h"
#endif

#if defined __linux__ || defined __FreeBSD__
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <regex>
#include <SDL.h>
#include <OgreFileSystemLayer.h>
#include "Logger/Log.h"
#include "filesystem/path.h"
#include "git_version.h"

#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

#include <OgreStringConverter.h>

namespace AV {
    /**
    Store the HLMS library locations in a format which can be read from system setup.
    This prevents me having to include the <map> namespace in the SystemSettings header.
     */
    std::map<std::string, SystemSetup::HlmsParams> intermediateHlmsLibraries;

    void SystemSetup::setup(const std::vector<std::string>& args){
        //Start by finding the master path.
        #ifdef TARGET_ANDROID
            //Android resources are relative to the apk bundle.
            SystemSettings::_masterPath = "";
        #else
            //This will be the pwd on most platforms, and the resources in the app bundle on mac.
            //I'm using SDL to find the bundle path or pwd.
            char *base_path = SDL_GetBasePath();
            SystemSettings::_masterPath = std::string(base_path);
            SDL_free(base_path);
        #endif


        memset(&SystemSettings::mUserComponentSettings, 0, sizeof(UserComponentSettings));

        {
            std::stringstream ss;
            ss << "** AVEngine Version "
            << ENGINE_VERSION_MAJOR << "."
            << ENGINE_VERSION_MINOR << "."
            << ENGINE_VERSION_PATCH << " "
            << ENGINE_VERSION_SUFFIX << " "
            << kGitHash << " **";
            const std::string title(ss.str());
            static const std::string separator(title.length(), '*');

            AV_INFO(separator);
            AV_INFO(title);
            AV_INFO(separator);

            bool testMode = false;
        #ifdef TEST_MODE
            testMode = true;
        #endif

            bool debugTools = false;
        #ifdef DEBUGGING_TOOLS
            debugTools = true;
        #endif

            AV_INFO("Engine features");
            AV_INFO("    Test mode available: {}", testMode);
            AV_INFO("    Debugging tools: {}", debugTools);
            AV_INFO(separator);
        }

        const ParsedArgs parsedArgs = _parseArguments(args);

        _determineAvSetupFiles(parsedArgs.positional);
        _determineUserSettingsFile();

        _determineUserDirectory();

        AV_INFO("Data path set to: " + SystemSettings::getDataPath());

        _processDataDirectory();
        UserSettingsSetup::processUserSettingsFile();

        _determineAvailableRenderSystems();
        SystemSettings::mCurrentRenderSystem = _determineRenderSystem(parsedArgs);

#ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()){
            AV_INFO("Test " + SystemSettings::getTestName() + " running.");
        }
#endif
    }

    SystemSetup::ParsedArgs SystemSetup::_parseArguments(const std::vector<std::string>& args){
        SystemSetup::ParsedArgs result;

        for(size_t i = 1; i < args.size(); i++){
            if(args[i].rfind("--", 0) == 0){ // Starts with "--"
                std::string key = args[i].substr(2);
                std::string value = (i + 1 < args.size() && args[i + 1].rfind("--", 0) != 0) ? args[++i] : "";
                result.optional[key] = value;
            }else{
                result.positional.push_back(args[i]);
            }
        }

        return result;
    }

    void SystemSetup::_determineAvSetupFiles(const std::vector<std::string>& args){
        int numSuccessFiles = 0;
        if(!args.empty()){
            for(int i = 0; i < args.size(); i++){
                bool success = _processSetupFilePath(args[i], &numSuccessFiles);
                if(success){
                    //Don't bother checking for a secondary file if the first was not found.
                    _processSetupFilePath((FilePath(args[i]).parent_path() / FilePath("avSetupSecondary.cfg")).str(), &numSuccessFiles);
                }
            }
        }else{
            //Default value if the provided path was broken, or just not provided.
            FilePath retPath = FilePath(SystemSettings::getMasterPath()) / FilePath("avSetup.cfg");
            bool success = _processSetupFilePath(retPath.str(), &numSuccessFiles);
            if(success){
                retPath = FilePath(SystemSettings::getMasterPath()) / FilePath("avSetupSecondary.cfg");
                _processSetupFilePath(retPath.str(), &numSuccessFiles);
            }
        }

        if(numSuccessFiles == 0){
            AV_ERROR("No valid avSetup.cfg files were found.");
        }
        SystemSettings::_avSetupFileViable = (numSuccessFiles > 0);
    }

    void SystemSetup::_determineUserDirectory(){
        std::string basePath;
        #ifdef __APPLE__
            basePath = GetApplicationSupportDirectory();
            //There is a chance Application Support won't exist for ios
            FilePath checkBasePath(basePath);
            if(!checkBasePath.exists()){
                std::filesystem::create_directory(checkBasePath.getStdPath());
            }
        #elif defined(TARGET_ANDROID)
            char* sdlBasePath = SDL_GetPrefPath("com.othermythos", "av");

            basePath = sdlBasePath;

            SDL_free(sdlBasePath);
        #elif (defined __linux__ || defined __FreeBSD__) && !defined(TARGET_ANDROID)
            const char *homedir;

            if ((homedir = getenv("HOME")) == NULL) {
                homedir = getpwuid(getuid())->pw_dir;
            }

            basePath = std::string(homedir) + "/.local/share";
        #elif _WIN32
            char* appdata = std::getenv("APPDATA");
            basePath = std::string(appdata);
        #endif

        filesystem::path testPath(basePath);
        testPath = testPath / "av";

        if(!testPath.exists()){
            filesystem::create_directory(testPath);
        }

        //Now the engine base path has been determined, create an enclosing directory for the current project.
        std::string projectName = SystemSettings::getProjectName();
        if(projectName.empty()){
            projectName = "empty";
        }
        testPath = testPath / projectName;

        SystemSettings::_userDirectoryPath = testPath.str();
        const char* delimChar = testPath.native_path == filesystem::path::path_type::posix_path ? "/" : "\\";
        SystemSettings::_userDirectoryPath.append(delimChar);

        AV_INFO("User path set to: {}", SystemSettings::_userDirectoryPath);
    }

    bool SystemSetup::_processSetupFilePath(const std::string& avFilePath, int* validFiles){
        //TODO OPTIMISATION there's lots of converting between string and paths if no path is provied.
        const FilePath setupPath(avFilePath);
        if(!setupPath.exists() || !setupPath.is_file()){
            AV_INFO("No avSetup.cfg file was found at the path {}.", avFilePath);
            return false;
        }

        AV_INFO("Setup file found {}", avFilePath);

        SystemSettings::_avSetupFilePath = setupPath.parent_path().str();

        _processAVSetupFile(avFilePath);

        (*validFiles)++;

        return true;
    }

    void SystemSetup::_determineAvailableRenderSystems(){
        #ifdef __APPLE__
            SystemSettings::mAvailableRenderSystems = {
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL
            };
        #elif __linux__ || __FreeBSD__
            SystemSettings::mAvailableRenderSystems = {
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL,
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_VULKAN
            };
        #elif _WIN32
            SystemSettings::mAvailableRenderSystems = {
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL,
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_VULKAN,
                SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11,
            };
        #endif
    }


    bool caseInsensitiveCompare(const std::string& str1, const std::string& str2) {
        return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(),
                          [](char a, char b) { return std::tolower(a) == std::tolower(b); });
    }
    SystemSettings::RenderSystemTypes SystemSetup::_determineRenderSystemForString(const std::string& rendersystem){
        static const std::array<SystemSettings::RenderSystemTypes, 4> VALS = {
            SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11,
            SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL,
            SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL,
            SystemSettings::RenderSystemTypes::RENDER_SYSTEM_VULKAN,
        };
        static std::array<std::string, 4> STRINGS = {
            "D3D11",
            "Metal",
            "OpenGL",
            "Vulkan",
        };

        for(size_t i = 0; i < STRINGS.size(); i++){
            if(caseInsensitiveCompare(rendersystem, STRINGS[i])){
                return VALS[i];
            }
        }

        return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET;
    }

    bool SystemSetup::_determineRenderSystemAvailable(SystemSettings::RenderSystemTypes renderSystem){
        const SystemSettings::RenderSystemContainer& c = SystemSettings::mAvailableRenderSystems;
        return std::find(c.begin(), c.end(), renderSystem) != c.end();
    }

    SystemSettings::RenderSystemTypes SystemSetup::_determineRenderSystem(const ParsedArgs& args){
        auto it = args.optional.find("rendersystem");
        if(it != args.optional.end()){
            const std::string value = it->second;
            SystemSettings::RenderSystemTypes renderType = _determineRenderSystemForString(value);
            if(renderType != SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET){
                if(_determineRenderSystemAvailable(renderType)){
                    return renderType;
                }
            }
        }

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
        if(rs == "Metal"){
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_METAL;
        }else if(rs == "OpenGL"){
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_OPENGL;
        }else if(rs == "Direct3D11"){
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_D3D11;
        }else if(rs == "Vulkan"){
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_VULKAN;
        }

        return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET;
    }

    void SystemSetup::_determineUserSettingsFile(){
        FilePath userSettingsFile = FilePath(SystemSettings::getMasterPath()) / FilePath("avUserSettings.cfg");
        auto thing = userSettingsFile.str();
        if(userSettingsFile.exists()){
            AV_INFO("User settings file found at path {}", userSettingsFile.str());

            SystemSettings::_userSettingsFileViable = true;
            SystemSettings::mUserSettingsFilePath = userSettingsFile.str();
        }else{
            AV_INFO("No user settings file was found in the master directory.")
        }
    }

    bool SystemSetup::_processAVSetupFile(const std::string& filePath){
        FILE* fp = fopen(filePath.c_str(), "r");
        if(fp == 0){
            AV_ERROR("Unable to open handle to file {}", filePath);
            return false;
        }
        char readBuffer[65536];
        rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
        rapidjson::Document d;
        d.ParseStream(is);
        fclose(fp);

        if(d.HasParseError()){
            AV_ERROR("Error parsing the setup file.");
            AV_ERROR(rapidjson::GetParseError_En(d.GetParseError()));

            return false;
        }

        _processAVSetupDocument(d);

        return true;
    }

    bool SystemSetup::_processAVSetupDocument(rapidjson::Document& d){
        using namespace rapidjson;

        Value::ConstMemberIterator itr;

        //Parse values.
        {
            itr = d.FindMember("WindowTitle");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::_windowTitle = itr->value.GetString();
            }
            itr = d.FindMember("Project");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::mProjectName = itr->value.GetString();
            }

            itr = d.FindMember("DataDirectory");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                //TODO move this somewhere else.
                const char* value = itr->value.GetString();

                filesystem::path dataDirectoryPath(value);
                const char* delimChar = dataDirectoryPath.native_path == filesystem::path::path_type::posix_path ? "/" : "\\";
                if(dataDirectoryPath.is_absolute()){
                    //If the user is providing an absolute path then just go with that.
                    if(dataDirectoryPath.exists()) {
                        SystemSettings::_dataPath = value;
                        SystemSettings::_dataPath.append(delimChar);
                    }
                    else AV_WARN("The data directory path provided ({}) in the avSetup.cfg file is not valid.", value);
                }else{
                    //The path is relative
                    //Find it as an absolute path for later.
                    filesystem::path p = filesystem::path(SystemSettings::getAvSetupFilePath()) / filesystem::path(value);
                    if(p.exists()){
                        SystemSettings::_dataPath = p.make_absolute().str();
                        //Append a directory delimiter to the end of the path.
                        SystemSettings::_dataPath.append(delimChar);
                    }else{
                        AV_WARN("The data directory path provided ({}) in the avSetup.cfg file is not valid.", value);
                    }
                }
            }

            itr = d.FindMember("CompositorBackground");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::_compositorColour = Ogre::StringConverter::parseColourValue(itr->value.GetString());
            }
            itr = d.FindMember("ResourcesFile");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::_ogreResourcesFilePath = itr->value.GetString();
            }
            itr = d.FindMember("SquirrelEntryFile");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::_squirrelEntryScriptPath = itr->value.GetString();
            }
            itr = d.FindMember("OgreResourcesFile");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::_ogreResourcesFilePath = itr->value.GetString();
            }
            itr = d.FindMember("MapsDirectory");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::mMapsDirectory = itr->value.GetString();
            }
            itr = d.FindMember("SaveDirectory");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::mSaveDirectory = itr->value.GetString();
            }
            itr = d.FindMember("WorldSlotSize");
            if(itr != d.MemberEnd() && itr->value.IsInt()){
                SystemSettings::_worldSlotSize = itr->value.GetInt();
            }
            #ifdef TEST_MODE
                itr = d.FindMember("TestMode");
                if(itr != d.MemberEnd() && itr->value.IsBool()){
                    SystemSettings::mTestModeEnabled = itr->value.GetBool();
                }
                itr = d.FindMember("TestName");
                if(itr != d.MemberEnd() && itr->value.IsString()){
                    SystemSettings::mTestName = itr->value.GetString();
                }
                itr = d.FindMember("TestTimeout");
                if(itr != d.MemberEnd() && itr->value.IsInt()){
                    SystemSettings::mTestModeTimeout = itr->value.GetInt();
                }
                itr = d.FindMember("TestTimeoutMeansFailure");
                if(itr != d.MemberEnd() && itr->value.IsBool()){
                    SystemSettings::mTimeoutMeansFail = itr->value.GetBool();
                }
            #endif

            itr = d.FindMember("WindowResizable");
            if(itr != d.MemberEnd() && itr->value.IsBool()){
                SystemSettings::mWindowResizable = itr->value.GetBool();
            }
            itr = d.FindMember("WindowWidth");
            if(itr != d.MemberEnd() && itr->value.IsInt()){
                _processWindowSize(SystemSettings::mDefaultWindowWidth, itr->value.GetInt());
            }
            itr = d.FindMember("WindowHeight");
            if(itr != d.MemberEnd() && itr->value.IsInt()){
                _processWindowSize(SystemSettings::mDefaultWindowHeight, itr->value.GetInt());
            }
            itr = d.FindMember("UseDefaultLights");
            if(itr != d.MemberEnd() && itr->value.IsBool()){
                SystemSettings::mUseDefaultLights = itr->value.GetBool();
            }
            itr = d.FindMember("DialogScript");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::mDialogImplementationScript = itr->value.GetString();
            }
            itr = d.FindMember("UseDefaultActionSet");
            if(itr != d.MemberEnd() && itr->value.IsBool()){
                SystemSettings::mUseDefaultActionSet = itr->value.GetBool();
            }
            itr = d.FindMember("UseDefaultCompositor");
            if(itr != d.MemberEnd() && itr->value.IsBool()){
                SystemSettings::mUseDefaultCompositor = itr->value.GetBool();
            }

            itr = d.FindMember("UserSettings");
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                _processSettingsFileUserEntries(itr->value);
            }
            itr = d.FindMember("Gui");
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                _processGuiSettings(itr->value);
            }
            itr = d.FindMember("OgreResources");
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                _processOgreResources(itr->value);
            }
            itr = d.FindMember("Plugins");
            if(itr != d.MemberEnd() && itr->value.IsArray()){
                _processPlugins(itr->value);
            }
            itr = d.FindMember("HLMS");
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                _processHlmsValues(itr->value);
            }
            itr = d.FindMember("DialogConstants");
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                _processDialogConstants(itr->value);
            }
            itr = d.FindMember("DynamicPhysics");
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                _parseDynamicWorldSettings(itr->value);
            }
            itr = d.FindMember("NumWorkerThreads");
            if(itr != d.MemberEnd() && itr->value.IsInt()){
                int val = itr->value.GetInt();
                //Set to the default.
                if(val <= 0 || val > 4) val = SystemSettings::mNumWorkerThreads;
                SystemSettings::mNumWorkerThreads = static_cast<char>(val);
            }
            itr = d.FindMember("Components");
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                _parseComponentSettings(itr->value);
            }
        }

        _parseCollisionWorldSettings(d);

        //Check whether physics has been completely turned off by the user.
        if(SystemSettings::getNumCollisionWorlds() <= 0 && SystemSettings::getDynamicPhysicsDisabled()){
            SystemSettings::mPhysicsCompletedDisabled = true;
        }

        return true;
    }

    void SystemSetup::_parseComponentSettings(const rapidjson::Value& parent){
        using namespace rapidjson;

        UserComponentSettings& foundSettings = SystemSettings::mUserComponentSettings;
        memset(&foundSettings, 0, sizeof(UserComponentSettings));
        for(Value::ConstMemberIterator itr = parent.MemberBegin(); itr != parent.MemberEnd(); ++itr){
            if(!itr->value.IsArray()) continue;
            const char* key = itr->name.GetString();

            if(itr->value.Size() > MAX_COMPONENT_DATA_TYPES){
                AV_ERROR("Too many variables defined in user component '{}'", key);
                continue;
            }
            //Iterate the variable type array.

            uint8 foundVariablesCounter = 0;
            ComponentDataTypes foundVars[MAX_COMPONENT_DATA_TYPES];
            for(Value::ConstValueIterator memItr = itr->value.Begin(); memItr != itr->value.End(); ++memItr){
                const rapidjson::Value& arrayVal = *memItr;
                if(!arrayVal.IsString()) continue;

                const char* varName = memItr->GetString();
                ComponentDataTypes foundType = ComponentDataTypes::NONE;
                if(strcmp(varName, "int") == 0){ foundType = ComponentDataTypes::INT; }
                else if(strcmp(varName, "float") == 0){ foundType = ComponentDataTypes::FLOAT; }
                else if(strcmp(varName, "bool") == 0){ foundType = ComponentDataTypes::BOOL; }

                if(foundType == ComponentDataTypes::NONE){
                    AV_ERROR("Read invalid value '{}' from component definition '{}'", varName, key);
                    continue;
                }else{
                    //Found a valid variable.
                    foundVars[foundVariablesCounter] = foundType;
                    foundVariablesCounter++;
                }
            }
            //Nothing to add to the list.
            if(foundVariablesCounter == 0){
                AV_ERROR("Read empty component definition.");
                continue;
            }

            //Push the component definition to the list.
            UserComponentSettings::ComponentSetting& setting = foundSettings.vars[foundSettings.numRegisteredComponents];
            setting.numVars = foundVariablesCounter;
            setting.componentName = key;
            setting.componentVars = _dataTypesToCombination(foundVars);

            foundSettings.numRegisteredComponents++;
        }

        //memcpy(&(SystemSettings::mUserComponentSettings), &foundSettings, sizeof(UserComponentSettings));
        //SystemSettings::mUserComponentSettings = foundSettings;
    }

    void SystemSetup::_parseDynamicWorldSettings(const rapidjson::Value& d){
        using namespace rapidjson;

        Value::ConstMemberIterator itr = d.FindMember("disabled");
        if(itr != d.MemberEnd() && itr->value.IsBool()){
            SystemSettings::mDynamicPhysicsDisabled = itr->value.GetBool();
        }
    }

    void SystemSetup::_parseCollisionWorldSettings(rapidjson::Document& d){
        using namespace rapidjson;

        uint8 numCollisionWorlds = 0;

        char c[50];
        for(int i = 0; i < MAX_COLLISION_WORLDS; i++){
            snprintf(c, 50, "Collision%i", i);
            Value::ConstMemberIterator itr = d.FindMember(c);
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                numCollisionWorlds++;
                if(numCollisionWorlds <= i){
                    //A gap was found in the ordering of the collision worlds.
                    numCollisionWorlds = 0;
                    break;
                }
            }
        }
        //Read the names and stuff from the list. Set this as the setting.
        SystemSettings::mNumberCollisionWorlds = numCollisionWorlds;
    }

    void SystemSetup::_processDialogConstants(const rapidjson::Value &val){
        using namespace rapidjson;

        for(Value::ConstMemberIterator itr = val.MemberBegin(); itr != val.MemberEnd(); ++itr){
            const char* key = itr->name.GetString();

            ConstantVariableAttribute var;
            if(itr->value.IsInt()){
                var.a.i = itr->value.GetInt();
                var.a._varData = static_cast<char>(AttributeType::INT);
            }
            else if(itr->value.IsDouble()){
                var.a.f = itr->value.GetDouble();
                var.a._varData = static_cast<char>(AttributeType::FLOAT);
            }
            else if(itr->value.IsBool()){
                var.a.f = itr->value.GetBool();
                var.a._varData = static_cast<char>(AttributeType::BOOLEAN);
            }
            else if(itr->value.IsString()){
                var.s = itr->value.GetString();
                var.a._varData = static_cast<char>(AttributeType::STRING);
            }else{
                //Ignore it.
                continue;
            }

            DialogSettings::mDialogConstantsMap[key] = var;
        }
    }

    void SystemSetup::_processPlugins(const rapidjson::Value &val){
        using namespace rapidjson;

        for(int i = 0; i < val.Size(); i++){
            const Value& entry = val[i];
            if(!entry.IsObject()) continue;

            Value::ConstMemberIterator itr = entry.FindMember("name");
            if(itr == entry.MemberEnd() || !itr->value.IsString()){
                AV_ERROR("Requested plugin does not properly define a name.");
                continue;
            }
            const char* pluginName = itr->value.GetString();

            Value::ConstMemberIterator pathItr = entry.FindMember("path");
            if(pathItr == entry.MemberEnd() || !(pathItr->value.IsString() || pathItr->value.IsArray())){
                AV_ERROR("Requested plugin does not properly define a path.");
                continue;
            }
            std::vector<std::string> pluginPaths;
            if(pathItr->value.IsString()){
                const char* pluginPath = pathItr->value.GetString();
                pluginPaths.push_back(pluginPath);
            }else{
                assert(pathItr->value.IsArray());
                for(int p = 0; p < pathItr->value.Size(); p++){
                    const Value& pp = pathItr->value[p];
                    if(!pp.IsString()) continue;
                    pluginPaths.push_back(pp.GetString());
                }
            }

            SystemSettings::mPluginEntries.push_back({pluginName, std::move(pluginPaths)});
        }

    }

    void SystemSetup::_processOgreResources(const rapidjson::Value &val){
        using namespace rapidjson;

        for(Value::ConstMemberIterator itr = val.MemberBegin(); itr != val.MemberEnd(); ++itr){
            const char* key = itr->name.GetString();
            if(strcmp(key, "File") == 0 && itr->name.IsString()){
                SystemSettings::_ogreResourcesFilePath = itr->value.GetString();
                continue;
            }
            if(!itr->value.IsArray()) continue;

            for(Value::ConstValueIterator memItr = itr->value.Begin(); memItr != itr->value.End(); ++memItr){
                if(!memItr->IsArray()) continue;
                if(memItr->Size() != 2) continue;
                const rapidjson::Value& arrayVal = *memItr;
                if(!arrayVal[0].IsString() || !arrayVal[1].IsString()) continue;

                const char* groupName = itr->name.GetString();
                const char* fsType = arrayVal[0].GetString(); //Currently not used but might be in future.
                const char* path = arrayVal[1].GetString();

                bool pathViable = false;
                std::string outPath;
                if(!_findDirectory(path, &pathViable, &outPath)){
                    AV_WARN("Ogre resource directory at {} does not exist", outPath);
                    continue;
                }

                _addOgreResourceLocation(groupName, outPath);
            }
        }
    }

    SystemSetup::HlmsParams* _getHlmsParamsForKey(const char* key){
        auto it = intermediateHlmsLibraries.find(key);
        SystemSetup::HlmsParams* target = 0;
        //Populate with the list
        if(it == intermediateHlmsLibraries.end()){
            //intermediateHlmsLibraries.insert(key, std::vector<std::string>());
            intermediateHlmsLibraries[key] = SystemSetup::HlmsParams();
            target = &intermediateHlmsLibraries[key];
        }else{
            target = &it->second;
        }

        return target;
    }
    void SystemSetup::_processHlmsValues(const rapidjson::Value &val){
        using namespace rapidjson;

        for(Value::ConstMemberIterator itr = val.MemberBegin(); itr != val.MemberEnd(); ++itr){
            const char* key = itr->name.GetString();
            const rapidjson::Value& innerVal = itr->value;

            for(Value::ConstMemberIterator innerItr = innerVal.MemberBegin(); innerItr != innerVal.MemberEnd(); ++innerItr){
                const char* innerKey = innerItr->name.GetString();
                if(strcmp(innerKey, "library") == 0){
                    if(innerItr->value.IsArray()){

                        for(int i = 0; i < innerItr->value.Size(); i++){
                            const rapidjson::Value& hlmsValue = innerItr->value[i];
                            if(!hlmsValue.IsString()) continue;

                            //Resolve the path relative to the hlms directory.
                            bool pathViable = false;
                            std::string outPath;
                            if(!_findDirectory(hlmsValue.GetString(), &pathViable, &outPath)){
                                AV_WARN("HLMS directory at {} does not exist", outPath);
                                continue;
                            }

                            HlmsParams* target = _getHlmsParamsForKey(key);
                            target->library.push_back(outPath);
                        }

                    }
                }
                else if(strcmp(innerKey, "template") == 0){
                    if(innerItr->value.IsString()){
                        std::string templatePath;
                        bool pathViable = false;
                        if(!_findDirectory(innerItr->value.GetString(), &pathViable, &templatePath)){
                            AV_WARN("HLMS template directory at {} does not exist", templatePath);
                            continue;
                        }

                        HlmsParams* target = _getHlmsParamsForKey(key);
                        target->templatePath = templatePath;
                    }
                }
            }
        }
    }

    void SystemSetup::_addOgreResourceLocation(const char* groupName, const std::string& path){
        unsigned char targetVal = 255;

        for(int i = 0; i < SystemSettings::mResourceGroupNames.size(); i++){
            if(SystemSettings::mResourceGroupNames[i] == groupName){
                targetVal = i;
            }
        }

        if(targetVal >= 255){
            //That group does not exist, as it wasn't found.
            size_t numGroupNames = SystemSettings::mResourceGroupNames.size();
            //If the user ever tries to create more than 255 groups, then just return.
            //If the user is doing this they'll have bigger problems elsewhere.
            if(numGroupNames >= 255) return;
            targetVal = numGroupNames;
            SystemSettings::mResourceGroupNames.push_back(groupName);
        }

        SystemSettings::mResourceEntries.push_back({targetVal, path});
    }

    void SystemSetup::_processGuiSettings(const rapidjson::Value &val){
        using namespace rapidjson;
        Value::ConstMemberIterator foundValue;
        foundValue = val.FindMember("Fonts");
        if(foundValue != val.MemberEnd() && foundValue->value.IsArray()){
            const rapidjson::Value& fontVal = foundValue->value;
            for(int i = 0; i < fontVal.Size(); i++){
                const rapidjson::Value& arrayVal = fontVal[i];
                if(!arrayVal.IsArray()) continue;
                if(arrayVal.Size() != 3) continue;
                if(!arrayVal[0].IsString() || !arrayVal[1].IsString() || !arrayVal[2].IsString()) continue;

                SystemSettings::mFontSettings.push_back({
                    arrayVal[0].GetString(),
                    arrayVal[1].GetString(),
                    arrayVal[2].GetString(),
                });
            }
        }

        //Read skins
        foundValue = val.FindMember("Skins");
        if(foundValue != val.MemberEnd() && foundValue->value.IsArray()){
            const rapidjson::Value& skinsList = foundValue->value;
            for(int i = 0; i < skinsList.Size(); i++){
                const rapidjson::Value& skinEntry = skinsList[i];
                if(!skinEntry.IsString()) continue;

                SystemSettings::mSkinPaths.push_back(skinEntry.GetString());
            }
        }
    }

    void SystemSetup::_processSettingsFileUserEntries(const rapidjson::Value &val){
        using namespace rapidjson;
        for(Value::ConstMemberIterator itr = val.MemberBegin(); itr != val.MemberEnd(); ++itr){
            const char* key = itr->name.GetString();
            size_t end = strlen(key);
            if(end > 0){
                if(*key == '#') continue;
            }else{
                continue;
            }

            Type type = itr->value.GetType();
            switch(type){
                case kFalseType:
                case kTrueType:
                    SystemSettings::_writeBoolToUserSettings(key, itr->value.GetBool());
                    break;
                case kStringType:
                    SystemSettings::_writeStringToUserSettings(key, itr->value.GetString());
                    break;
                case kNumberType:{
                    if(itr->value.IsDouble()){
                        SystemSettings::_writeFloatToUserSettings(key, itr->value.GetDouble());
                    }else{
                        SystemSettings::_writeIntToUserSettings(key, itr->value.GetInt());
                    }
                    break;
                }
                default:
                    continue;
            }
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

        //Check fonts
        for(SystemSettings::FontSettingEntry& e : SystemSettings::mFontSettings){
            bool found = false;
            _findFile(found, e.fontPath);
        }
        //Check skins
        for(std::string& e : SystemSettings::mSkinPaths){
            bool found = false;
            _findFile(found, e);
        }
        for(const std::pair<std::string, HlmsParams>& itorPair : intermediateHlmsLibraries){
            for(const std::string& path : itorPair.second.library){
                bool found = false;
                std::string outPath;
                _findDirectory(path, &found, &outPath);
                if(!found){
                    AV_WARN("Could not find directory with path '{}' for HLMS library '{}'", outPath, path);
                }else{
                    SystemSettings::writeHlmsUserLibraryEntry(itorPair.first, outPath);
                }
            }
            if(!itorPair.second.templatePath.empty()){
                SystemSettings::writeHlmsUserTemplateEntry(itorPair.first, itorPair.second.templatePath);
            }
        }
        intermediateHlmsLibraries.clear();


        AV_INFO("OgreResourcesFile set to {}", SystemSettings::getOgreResourceFilePath());
        AV_INFO("SquirrelEntryFile set to {}", SystemSettings::getSquirrelEntryScriptPath());
        AV_INFO("Maps Directory set to {}", SystemSettings::getMapsDirectory());
        AV_INFO("Save Directory set to {}", SystemSettings::getSaveDirectory());
    }

    void SystemSetup::_findOgreResourcesFile(){
        if(!_findFile(SystemSettings::_ogreResourcesFileViable, SystemSettings::_ogreResourcesFilePath)){
            //TODO only warn about this if paths in the setup file haven't been provided as well.
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

        FilePath fPath(outPath);
        if(!fPath.is_absolute()){
            fPath = (FilePath(SystemSettings::getDataPath()) / fPath);
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

        FilePath dirPath(directory);
        if(!dirPath.is_absolute()){
            dirPath = (FilePath(SystemSettings::getDataPath()) / dirPath);
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
