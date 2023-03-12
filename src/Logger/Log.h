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
        inline static std::shared_ptr<spdlog::logger>& GetOgreLogger() { return _ogreLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetSquirrelLogger() { return _squirrelLogger; }
    private:
        static std::shared_ptr<spdlog::logger> _logger;
        static std::shared_ptr<spdlog::logger> _ogreLogger;
        static std::shared_ptr<spdlog::logger> _squirrelLogger;
    };
}

#define AV_TRACE(...) ::AV::Log::GetLogger()->trace(__VA_ARGS__);
#define AV_INFO(...) ::AV::Log::GetLogger()->info(__VA_ARGS__);
#define AV_WARN(...) ::AV::Log::GetLogger()->warn(__VA_ARGS__);
#define AV_ERROR(...) ::AV::Log::GetLogger()->error(__VA_ARGS__);
#define AV_CRITICAL(...) ::AV::Log::GetLogger()->critical(__VA_ARGS__);

#define AV_OGRE_TRACE(...) ::AV::Log::GetOgreLogger()->trace(__VA_ARGS__);
#define AV_OGRE_INFO(...) ::AV::Log::GetOgreLogger()->info(__VA_ARGS__);
#define AV_OGRE_WARN(...) ::AV::Log::GetOgreLogger()->warn(__VA_ARGS__);
#define AV_OGRE_ERROR(...) ::AV::Log::GetOgreLogger()->error(__VA_ARGS__);
#define AV_OGRE_CRITICAL(...) ::AV::Log::GetOgreLogger()->critical(__VA_ARGS__);

#define AV_SQUIRREL_PRINT(...) ::AV::Log::GetSquirrelLogger()->info(__VA_ARGS__);
