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


namespace AV {
    std::shared_ptr<spdlog::logger> Log::_logger;
    std::shared_ptr<spdlog::logger> Log::_ogreLogger;

    void Log::Init(){
        #ifdef WIN32
            #ifdef WIN_DESKTOP_APPLICATION
                auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
                _logger = std::make_shared<spdlog::logger>("AV", sink);
                _ogreLogger = std::make_shared<spdlog::logger>("OGRE", sink);
            #else
                _logger = spdlog::stdout_color_mt("AV");
                _ogreLogger = spdlog::stdout_color_mt("OGRE");
            #endif

        #else
            _logger = spdlog::stdout_color_mt("AV");
            _ogreLogger = spdlog::stdout_color_mt("OGRE");
        #endif

        _logger->set_level(spdlog::level::trace);
        _ogreLogger->set_level(spdlog::level::trace);
    }
}
