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
    
    void Log::Init(){
        #ifdef WIN32
            #ifdef WIN_DESKTOP_APPLICATION
                auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
                _logger = std::make_shared<spdlog::logger>("AV", sink);
            #else
                _logger = spdlog::stdout_color_mt("AV");
            #endif

        #else
            _logger = spdlog::stdout_color_mt("AV");
        #endif

        _logger->set_level(spdlog::level::trace);
    }
}
