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

        struct AVLogger{
            std::shared_ptr<spdlog::logger> _term;
            std::shared_ptr<spdlog::logger> _file;
        };

        inline static AVLogger& GetLogger() { return _logger; }
        inline static AVLogger& GetOgreLogger() { return _ogreLogger; }
        inline static AVLogger& GetSquirrelLogger() { return _squirrelLogger; }
    private:
        static AVLogger _logger;
        static AVLogger _ogreLogger;
        static AVLogger _squirrelLogger;

        static void _setupBasicLoggers(const char* filePath);
        static std::string _setupPathForPlatform();
    };
}

#define AV_TRACE(...) { ::AV::Log::GetLogger()._term->trace(__VA_ARGS__); ::AV::Log::GetLogger()._file->trace(__VA_ARGS__); }
#define AV_INFO(...) { ::AV::Log::GetLogger()._term->info(__VA_ARGS__); ::AV::Log::GetLogger()._file->info(__VA_ARGS__); }
#define AV_WARN(...) { ::AV::Log::GetLogger()._term->warn(__VA_ARGS__); ::AV::Log::GetLogger()._file->warn(__VA_ARGS__); }
#define AV_ERROR(...) { ::AV::Log::GetLogger()._term->error(__VA_ARGS__); ::AV::Log::GetLogger()._file->error(__VA_ARGS__); }
#define AV_CRITICAL(...) { ::AV::Log::GetLogger()._term->critical(__VA_ARGS__); ::AV::Log::GetLogger()._file->critical(__VA_ARGS__); }

#define AV_OGRE_TRACE(...) { ::AV::Log::GetOgreLogger()._term->trace(__VA_ARGS__); ::AV::Log::GetOgreLogger()._file->trace(__VA_ARGS__); }
#define AV_OGRE_INFO(...) { ::AV::Log::GetOgreLogger()._term->info(__VA_ARGS__); ::AV::Log::GetOgreLogger()._file->info(__VA_ARGS__); }
#define AV_OGRE_WARN(...) { ::AV::Log::GetOgreLogger()._term->warn(__VA_ARGS__); ::AV::Log::GetOgreLogger()._file->warn(__VA_ARGS__); }
#define AV_OGRE_ERROR(...) { ::AV::Log::GetOgreLogger()._term->error(__VA_ARGS__); ::AV::Log::GetOgreLogger()._file->error(__VA_ARGS__); }
#define AV_OGRE_CRITICAL(...) { ::AV::Log::GetOgreLogger()._term->critical(__VA_ARGS__); ::AV::Log::GetOgreLogger()._file->critical(__VA_ARGS__); }

#define AV_SQUIRREL_PRINT(...) { ::AV::Log::GetSquirrelLogger()._term->info(__VA_ARGS__); ::AV::Log::GetSquirrelLogger()._file->info(__VA_ARGS__); }
