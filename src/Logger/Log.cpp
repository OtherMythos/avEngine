#include "Log.h"

#ifdef WIN32
    #ifdef WIN_DESKTOP_APPLICATION
        #include "spdlog/sinks/msvc_sink.h"
    #else
        #include "spdlog/sinks/stdout_color_sinks.h"
    #endif
#else
    #include "spdlog/sinks/stdout_color_sinks.h"
#endif

#ifdef __APPLE__
    //TODO change this to be something else.
    #include "Window/SDL2Window/MacOS/MacOSUtils.h"
#endif

#if defined __linux__ || defined __FreeBSD__
    #include <unistd.h>
    #include <sys/types.h>
    #include <pwd.h>
#endif

#include "spdlog/sinks/basic_file_sink.h"

#include <filesystem>

namespace AV {
    Log::AVLogger Log::_logger;
    Log::AVLogger Log::_ogreLogger;
    Log::AVLogger Log::_squirrelLogger;

    void Log::Init(){
        std::string platformPath = _setupPathForPlatform();

        #ifdef WIN32
            #ifdef WIN_DESKTOP_APPLICATION
                auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
                _logger = std::make_shared<spdlog::logger>("AV", sink);
                _ogreLogger = std::make_shared<spdlog::logger>("OGRE", sink);
                _squirrelLogger = std::make_shared<spdlog::logger>("SQUIRREL", sink);
            #else
                _setupBasicLoggers();
            #endif

        #else
            _setupBasicLoggers(platformPath.c_str());
        #endif

        _logger._term->set_level(spdlog::level::trace);
        _ogreLogger._term->set_level(spdlog::level::trace);
        _squirrelLogger._term->set_level(spdlog::level::trace);
    }

    void Log::_setupBasicLoggers(const char* filePath){
        auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath);

        _logger = { spdlog::stdout_color_mt("AV"), std::make_shared<spdlog::logger>("AV", fileSink) };
        _ogreLogger = { spdlog::stdout_color_mt("OGRE"), std::make_shared<spdlog::logger>("OGRE", fileSink) };
        _squirrelLogger = { spdlog::stdout_color_mt("SQUIRREL"), std::make_shared<spdlog::logger>("SQUIRREL", fileSink) };
    }

    std::string Log::_setupPathForPlatform(){
        std::filesystem::path targetPath;
        #ifdef __APPLE__
            #ifdef TARGET_APPLE_IPHONE
            #else
                targetPath = std::filesystem::path(GetApplicationSupportDirectory()) / "../Logs";
                if(!std::filesystem::exists(targetPath)){
                    std::filesystem::create_directory(targetPath);
                }
                targetPath = std::filesystem::canonical(targetPath);
            #endif
        #elif defined __linux__ || defined __FreeBSD__
            const char *homedir;

            if ((homedir = getenv("HOME")) == NULL) {
                homedir = getpwuid(getuid())->pw_dir;
            }

            targetPath = std::filesystem::path(std::string(homedir) + "/.local/share/av/logs");
            if(!std::filesystem::exists(targetPath)){
                std::filesystem::create_directory(targetPath);
            }
            targetPath = std::filesystem::canonical(targetPath);
        #elif _WIN32
        #endif

        targetPath /= "av";
        if(!std::filesystem::exists(targetPath)){
            std::filesystem::create_directory(targetPath);
        }
        targetPath /= "av.log";

        return targetPath.string();
    }
}
