#include "SystemSetup.h"

#ifdef __APPLE__
    #include "Window/SDL2Window/MacOS/MacOSUtils.h"
#endif
#include "SystemSettings.h"
#include "System/SystemSetup/SystemSettings.h"
#include "System/EnginePrerequisites.h"
#include "Entity/UserComponents/UserComponentData.h"
#include "System/FileSystem/FilePath.h"
#include <filesystem>
#include "System/Util/FileSystemHelper.h"
#include "System/Util/PathUtils.h"

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

    /**
    Plugin directories as written in the avSetup files, before the data directory is known.
    These are resolved and read in _processPluginDirectories.
     */
    std::vector<std::string> intermediatePluginDirectories;

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

            bool debugServer = false;
        #ifdef DEBUG_SERVER
            debugServer = true;
        #endif

            bool scriptProfiler = false;
        #ifdef SCRIPT_PROFILER
            scriptProfiler = true;
        #endif

            AV_INFO("Engine features");
            AV_INFO("    Test mode available: {}", testMode);
            AV_INFO("    Debugging tools: {}", debugTools);
            AV_INFO("    Debug server available: {}", debugServer);
            AV_INFO("    Script profiler available: {}", scriptProfiler);
            AV_INFO(separator);
        }

        const ParsedArgs parsedArgs = _parseArguments(args);

        _determineAvSetupFiles(parsedArgs.positional);

        _determineUserDirectory();

        AV_INFO("Data path set to: " + SystemSettings::getDataPath());

        _processDataDirectory();

        _determineAvailableRenderSystems();
        SystemSettings::mCurrentRenderSystem = _determineRenderSystem(parsedArgs);

        _processArguments(parsedArgs);

#ifdef TEST_MODE
        if(SystemSettings::isTestModeEnabled()){
            AV_INFO("Test " + SystemSettings::getTestName() + " running.");
        }
#endif
    }

    static bool _isBooleanFlag(const std::string& key){
        return key == "headless" || key == "noDebugger" || key == "disableVsync";
    }

    static bool _looksLikeBoolean(const std::string& value){
        std::string lower;
        lower.reserve(value.size());
        for(char c : value) lower += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

        return lower == "true" || lower == "false" || lower == "yes" || lower == "no" ||
               lower == "1" || lower == "0";
    }

    SystemSetup::ParsedArgs SystemSetup::_parseArguments(const std::vector<std::string>& args){
        SystemSetup::ParsedArgs result;

        for(size_t i = 1; i < args.size(); i++){
            if(args[i].rfind("--", 0) == 0){ // Starts with "--"
                std::string key = args[i].substr(2);
                std::string value;
                if(i + 1 < args.size() && args[i + 1].rfind("--", 0) != 0){
                    const bool consume = !_isBooleanFlag(key) || _looksLikeBoolean(args[i + 1]);
                    if(consume) value = args[++i];
                }
                result.optional[key] = value;
            }else{
                result.positional.push_back(args[i]);
            }
        }

        return result;
    }

    std::string SystemSetup::getLogFilePathFromArgs(const std::vector<std::string>& args){
        const ParsedArgs parsedArgs = _parseArguments(args);

        auto it = parsedArgs.optional.find("logFile");
        if(it == parsedArgs.optional.end()) return "";

        return it->second;
    }

    void SystemSetup::_processArguments(const ParsedArgs& args){
        //For the boolean flags, the flag being present with no value means true.
        //See _isBooleanFlag.
        auto it = args.optional.find("disableVsync");
        if(it != args.optional.end()){
            SystemSettings::mForceDisableVsync = Ogre::StringConverter::parseBool(it->second, true);
        }

        auto noDebuggerIt = args.optional.find("noDebugger");
        if(noDebuggerIt != args.optional.end()){
            SystemSettings::mNoDebugger = Ogre::StringConverter::parseBool(noDebuggerIt->second, true);
        }

        auto headlessIt = args.optional.find("headless");
        if(headlessIt != args.optional.end() && Ogre::StringConverter::parseBool(headlessIt->second, true)){
        #if defined(TARGET_APPLE_IPHONE) || defined(TARGET_ANDROID)
            AV_WARN("--headless is a desktop only flag and was ignored.");
        #else
            SystemSettings::mHeadless = true;
            //Headless renders into an offscreen texture which is never presented, so
            //there is nothing to synchronise to. Frames free-run.
            SystemSettings::mForceDisableVsync = true;
            AV_INFO("Headless mode enabled. No window will be created.");
        #endif
        }

        //Overrides whatever the setup file says. Intended for test tooling: a script waiting on
        //N logic ticks (a grace period, a retry loop) reaches that count in N/FixedUpdateRate real
        //seconds, so raising the rate compresses tick-counted waits without touching the test.
        //It does NOT speed up anything keyed to real time - animation durations are fed
        //milliseconds that sum to one real second of playback per real second
        //elapsed, whatever the tick rate, and physics stays real-time regardless of PhysicsUpdateRate
        //for the same reason (see PhysicsThread).
        //
        //IMPORTANT: physics steps per script update = PhysicsUpdateRate / FixedUpdateRate. A test
        //asserting an exact collision event count per update (the "one physics step per update"
        //tests) relies on that ratio staying 1, so raising FixedUpdateRate alone breaks them -
        //raise --physicsUpdateRate by the same factor alongside it.
        //Same clamp as the matching setup file keys, so this never reaches untested territory.
        auto fixedUpdateRateIt = args.optional.find("fixedUpdateRate");
        if(fixedUpdateRateIt != args.optional.end()){
            int val = Ogre::StringConverter::parseInt(fixedUpdateRateIt->second, SystemSettings::mFixedUpdateRate);
            if(val > 0 && val <= 240){
                SystemSettings::mFixedUpdateRate = val;
            }else{
                AV_WARN("Invalid --fixedUpdateRate '{}', using {}.", fixedUpdateRateIt->second, SystemSettings::mFixedUpdateRate);
            }
        }

        auto physicsUpdateRateIt = args.optional.find("physicsUpdateRate");
        if(physicsUpdateRateIt != args.optional.end()){
            int val = Ogre::StringConverter::parseInt(physicsUpdateRateIt->second, SystemSettings::mPhysicsUpdateRate);
            if(val > 0 && val <= 240){
                SystemSettings::mPhysicsUpdateRate = val;
            }else{
                AV_WARN("Invalid --physicsUpdateRate '{}', using {}.", physicsUpdateRateIt->second, SystemSettings::mPhysicsUpdateRate);
            }
        }

#ifdef DEBUG_SERVER
        auto debugServerIt = args.optional.find("debugServer");
        if(debugServerIt != args.optional.end()){
            SystemSettings::mDebugServerEnabled = true;
            const std::string& portValue = debugServerIt->second;
            if(!portValue.empty()){
                int port = Ogre::StringConverter::parseInt(portValue, 0);
                if(port > 0 && port <= 65535){
                    SystemSettings::mDebugServerPort = port;
                }else{
                    AV_WARN("Invalid --debugServer port '{}', using default {}.", portValue, SystemSettings::mDebugServerPort);
                }
            }
        }
#endif

#ifdef SCRIPT_PROFILER
        auto profileScriptsIt = args.optional.find("profileScripts");
        if(profileScriptsIt != args.optional.end()){
            SystemSettings::mScriptProfilerEnabled = true;
            //The optional argument is where the full report is written at shutdown.
            SystemSettings::mScriptProfilerOutputPath = profileScriptsIt->second;
        }

        auto profileLinesIt = args.optional.find("profileScriptsLines");
        if(profileLinesIt != args.optional.end()){
            SystemSettings::mScriptProfilerLines = Ogre::StringConverter::parseBool(profileLinesIt->second, true);
        }
#endif
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
                AV_INFO("User requested rendersystem '{}'.", value);
                if(_determineRenderSystemAvailable(renderType)){
                    return renderType;
                }else{
                    AV_WARN("User requested rendersystem '{}' is unavailable on this system.", value);
                }
            }else{
                AV_WARN("User requested rendersystem '{}' is unrecognised.", value);
            }
        }

        const SystemSettings::RenderSystemContainer& available = SystemSettings::getAvailableRenderSystems();

        if(available.size() <= 0){
            //Call _determineAvailableRenderSystems() first.
            AV_ERROR("No available render systems have been registered.");
            return SystemSettings::RenderSystemTypes::RENDER_SYSTEM_UNSET;
        }

        //The first value in the vector is considered the default for the platform.
        //A project can override this from its Squirrel setup() function with
        //_settings.setRenderSystem(), which runs before Ogre is created.
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


    bool SystemSetup::_processAVSetupFile(const std::string& filePath){
        rapidjson::Document d;
        if(!FileSystemHelper::setupRapidJsonDocument(filePath.c_str(), &d)){
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
                SystemSettings::_ogreResourcesFileRequested = true;
            }
            itr = d.FindMember("SquirrelEntryFile");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::_squirrelEntryScriptPath = itr->value.GetString();
            }
            itr = d.FindMember("OgreResourcesFile");
            if(itr != d.MemberEnd() && itr->value.IsString()){
                SystemSettings::_ogreResourcesFilePath = itr->value.GetString();
                SystemSettings::_ogreResourcesFileRequested = true;
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
            itr = d.FindMember("UseDefaultActionSet");
            if(itr != d.MemberEnd() && itr->value.IsBool()){
                SystemSettings::mUseDefaultActionSet = itr->value.GetBool();
            }
            itr = d.FindMember("UseDefaultCompositor");
            if(itr != d.MemberEnd() && itr->value.IsBool()){
                SystemSettings::mUseDefaultCompositor = itr->value.GetBool();
            }
            itr = d.FindMember("DisableAudio");
            if(itr != d.MemberEnd() && itr->value.IsBool()){
                SystemSettings::mDisableAudio = itr->value.GetBool();
            }
            itr = d.FindMember("FixedUpdateRate");
            if(itr != d.MemberEnd() && itr->value.IsInt()){
                int val = itr->value.GetInt();
                if(val > 0 && val <= 240) SystemSettings::mFixedUpdateRate = val;
            }
            itr = d.FindMember("PhysicsUpdateRate");
            if(itr != d.MemberEnd() && itr->value.IsInt()){
                int val = itr->value.GetInt();
                if(val > 0 && val <= 240) SystemSettings::mPhysicsUpdateRate = val;
            }
            itr = d.FindMember("MaxPhysicsStepsPerUpdate");
            if(itr != d.MemberEnd() && itr->value.IsInt()){
                int val = itr->value.GetInt();
                if(val > 0 && val <= 32) SystemSettings::mMaxPhysicsStepsPerUpdate = val;
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
            itr = d.FindMember("ScriptWorkers");
            if(itr != d.MemberEnd() && itr->value.IsObject()){
                _parseScriptWorkerSettings(itr->value);
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

    void SystemSetup::_parseScriptWorkerSettings(const rapidjson::Value& d){
        using namespace rapidjson;

        Value::ConstMemberIterator itr = d.FindMember("enabled");
        if(itr != d.MemberEnd() && itr->value.IsBool()){
            SystemSettings::mScriptWorkersEnabled = itr->value.GetBool();
        }

        itr = d.FindMember("maxWorkers");
        if(itr != d.MemberEnd() && itr->value.IsInt()){
            int val = itr->value.GetInt();
            //Clamped rather than rejected, in the same spirit as NumWorkerThreads above.
            if(val < 1) val = 1;
            else if(val > 16) val = 16;
            SystemSettings::mMaxScriptWorkers = static_cast<uint8>(val);
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

    void SystemSetup::_processPlugins(const rapidjson::Value &val){
        using namespace rapidjson;

        //Only the directories are collected here. Resolving them needs the data directory, which
        //is not final until every setup file has been read, so that waits for
        //_processPluginDirectories.
        for(int i = 0; i < val.Size(); i++){
            const Value& entry = val[i];
            if(!entry.IsString()){
                AV_ERROR("Plugin entries should be a string path to a directory containing an avPlugin.cfg file.");
                continue;
            }

            intermediatePluginDirectories.push_back(entry.GetString());
        }
    }

    void SystemSetup::_processPluginDirectories(){
        for(const std::string& directory : intermediatePluginDirectories){
            //Plugin directories accept the res:// and user:// schemes, unlike most setup paths.
            std::string resolvedDirectory;
            formatResToPath(directory, resolvedDirectory);

            bool viable = false;
            std::string outDirectory;
            _findDirectory(resolvedDirectory, &viable, &outDirectory);
            if(!viable){
                AV_ERROR("No plugin directory was found at the path '{}'", outDirectory);
                continue;
            }

            SystemSettings::PluginEntry entry;
            entry.directory = outDirectory;
            if(!_processPluginFile(outDirectory, entry)) continue;

            AV_INFO("Found plugin '{}' at {}", entry.name, entry.directory);
            SystemSettings::mPluginEntries.push_back(std::move(entry));
        }

        intermediatePluginDirectories.clear();
    }

    bool SystemSetup::_processPluginFile(const std::string& pluginDirectory, SystemSettings::PluginEntry& outEntry){
        using namespace rapidjson;

        const FilePath cfgPath = FilePath(pluginDirectory) / FilePath("avPlugin.cfg");
        if(!cfgPath.exists() || !cfgPath.is_file()){
            AV_ERROR("No avPlugin.cfg file was found in the plugin directory '{}'", pluginDirectory);
            return false;
        }

        Document d;
        if(!FileSystemHelper::setupRapidJsonDocument(cfgPath.str().c_str(), &d)){
            return false;
        }

        Value::ConstMemberIterator itr = d.FindMember("Name");
        if(itr == d.MemberEnd() || !itr->value.IsString()){
            AV_ERROR("The plugin at '{}' does not define a Name.", pluginDirectory);
            return false;
        }
        outEntry.name = itr->value.GetString();

        itr = d.FindMember("Description");
        if(itr != d.MemberEnd() && itr->value.IsString()){
            outEntry.description = itr->value.GetString();
        }

        itr = d.FindMember("Version");
        if(itr != d.MemberEnd() && itr->value.IsString()){
            outEntry.version = itr->value.GetString();
        }

        //A script plugin. The entry file is a callback script, so it can declare start, update,
        //end and sceneSafeUpdate functions.
        itr = d.FindMember("EntryFile");
        if(itr != d.MemberEnd() && itr->value.IsString()){
            const FilePath entryPath = FilePath(pluginDirectory) / FilePath(itr->value.GetString());
            if(!entryPath.exists() || !entryPath.is_file()){
                AV_ERROR("The plugin '{}' declares an EntryFile which does not exist at '{}'", outEntry.name, entryPath.str());
                return false;
            }
            outEntry.entryFile = entryPath.str();
        }

        //A native plugin. Each path is a candidate searched in order, and may be either a
        //directory to scan or a direct path to a library.
        itr = d.FindMember("Bin");
        if(itr != d.MemberEnd() && itr->value.IsObject()){
            Value::ConstMemberIterator pathItr = itr->value.FindMember("Path");
            if(pathItr == itr->value.MemberEnd() || !(pathItr->value.IsString() || pathItr->value.IsArray())){
                AV_ERROR("The plugin '{}' declares a Bin section without a Path.", outEntry.name);
                return false;
            }
            if(pathItr->value.IsString()){
                outEntry.binPaths.push_back((FilePath(pluginDirectory) / FilePath(pathItr->value.GetString())).str());
            }else{
                for(int p = 0; p < pathItr->value.Size(); p++){
                    const Value& pp = pathItr->value[p];
                    if(!pp.IsString()) continue;
                    outEntry.binPaths.push_back((FilePath(pluginDirectory) / FilePath(pp.GetString())).str());
                }
            }
        }

        if(outEntry.entryFile.empty() && outEntry.binPaths.empty()){
            AV_WARN("The plugin '{}' declares neither an EntryFile nor a Bin section, so nothing will be loaded for it.", outEntry.name);
        }

        return true;
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
                        #ifndef TARGET_ANDROID
                            std::string templatePath;
                            bool pathViable = false;
                            if(!_findDirectory(innerItr->value.GetString(), &pathViable, &templatePath)){
                                AV_WARN("HLMS template directory at {} does not exist", templatePath);
                                continue;
                            }
                        #else
                            //Android makes it quite hard to check if a directory exists within the bundle.
                            //Just assume the path is ok.
                            std::string templatePath = innerItr->value.GetString();
                            bool pathViable = true;
                        #endif

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
        //Needs the data directory, so it can't happen while the setup files are being read.
        _processPluginDirectories();

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


        AV_INFO("SquirrelEntryFile set to {}", SystemSettings::getSquirrelEntryScriptPath());
    }

    void SystemSetup::_findOgreResourcesFile(){
        //Resource locations can be registered by other means, so only mention the file if one was explicitly requested but not found.
        if(!_findFile(SystemSettings::_ogreResourcesFileViable, SystemSettings::_ogreResourcesFilePath)
                && SystemSettings::_ogreResourcesFileRequested){
            AV_INFO("No OgreResources file was found at path {}.", SystemSettings::_ogreResourcesFilePath);
        }
    }

    void SystemSetup::_findSquirrelEntryFile(){
        if(!_findFile(SystemSettings::_squirrelEntryScriptViable, SystemSettings::_squirrelEntryScriptPath)){
            AV_WARN("The Squirrel entry file provided ({}) in the avSetup.cfg file is not valid.", SystemSettings::_squirrelEntryScriptPath);
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


}
