#include "Log.h"

#include "spdlog/sinks/stdout_color_sinks.h"

namespace AV {
    std::shared_ptr<spdlog::logger> Log::_logger;
    
    void Log::Init(){
        _logger = spdlog::stdout_color_mt("AV");
        _logger->set_level(spdlog::level::trace);
    }
}
