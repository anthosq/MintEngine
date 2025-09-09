#pragma once

#include <spdlog/spdlog.h>

#include <cstdint>
#include <stdexcept>
#include <memory>

// considering moving to macro.h later
// usage: LOG(LogSystem::LogLevel::info, "Hello {}!", "world");
#include "global_context.h"
#define LOG_HELPER(LOG_LEVEL, ...) \
    g_runtime_global_context.m_log_system->log(LOG_LEVEL, "[" + std::string(__FUNCTION__) + "] " +  __VA_ARGS__);

#define LOG_DEBUG(...) LOG_HELPER(Mint::LogSystem::LogLevel::debug, __VA_ARGS__)
#define LOG_INFO(...)  LOG_HELPER(Mint::LogSystem::LogLevel::info,  __VA_ARGS__)
#define LOG_WARN(...)  LOG_HELPER(Mint::LogSystem::LogLevel::warn,  __VA_ARGS__)
#define LOG_ERROR(...) LOG_HELPER(Mint::LogSystem::LogLevel::error, __VA_ARGS__)
#define LOG_FATAL(...) LOG_HELPER(Mint::LogSystem::LogLevel::fatal, __VA_ARGS__)

// ---------------------------------------------------
namespace Mint {
    class LogSystem {
        public:
            enum class LogLevel {
                debug,
                info,
                warn,
                error,
                fatal
            };

        public:
            LogSystem();
            ~LogSystem();

            template <class... TARGS>
            void log(LogLevel level, TARGS&&... args) {
                switch (level) {
                    case LogLevel::debug:
                        m_logger->debug(std::forward<TARGS>(args)...);
                        break;
                    case LogLevel::info:
                        m_logger->info(std::forward<TARGS>(args)...);
                        break;
                    case LogLevel::warn:
                        m_logger->warn(std::forward<TARGS>(args)...);
                        break;
                    case LogLevel::error:
                        m_logger->error(std::forward<TARGS>(args)...);
                        break;
                    case LogLevel::fatal:
                        m_logger->critical(std::forward<TARGS>(args)...);
                        break;
                    default:
                        // throw std::runtime_error("Unknown log level");
                        break;
                }
            }

            template <class ...TARGS>
            void fatalCallback(TARGS&&... args) {
                const std::string msg = fmt::format(std::forward<TARGS>(args)...);
                throw std::runtime_error(msg);
            }

        private:
            std::shared_ptr<spdlog::logger> m_logger;
    };
}