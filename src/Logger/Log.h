#pragma once
#include "iostream"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace AV {
    class Log{
    public:
        /**
         Initialises the logging system.
         
         @remarks This should be one of the first functions called during startup.
         */
        static void Init();
        
        inline static std::shared_ptr<spdlog::logger>& GetLogger() { return _logger; }
    private:
        static std::shared_ptr<spdlog::logger> _logger;
    };
}

#define AV_TRACE(...) ::AV::Log::GetLogger()->trace(__VA_ARGS__);
#define AV_INFO(...) ::AV::Log::GetLogger()->info(__VA_ARGS__);
#define AV_WARN(...) ::AV::Log::GetLogger()->warn(__VA_ARGS__);
#define AV_ERROR(...) ::AV::Log::GetLogger()->error(__VA_ARGS__);
#define AV_CRITICAL(...) ::AV::Log::GetLogger()->critical(__VA_ARGS__);
