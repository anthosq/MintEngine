#include <spdlog/spdlog.h>
#include "log_system.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

namespace Mint {
    LogSystem::LogSystem() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        console_sink->set_pattern("[%T] [%^%l%$] %v");

        const spdlog::sinks_init_list sink_list = {console_sink};
        spdlog::init_thread_pool(8192, 1);

        m_logger = std::make_shared<spdlog::async_logger>("MintLogger",
                                                         sink_list.begin(),
                                                         sink_list.end(),
                                                         spdlog::thread_pool(), 
                                                         spdlog::async_overflow_policy::block);
        m_logger->set_level(spdlog::level::trace);

        spdlog::register_logger(m_logger);
    }

    LogSystem::~LogSystem() {
        m_logger->flush();
        spdlog::drop_all();
    }
}

// try {
//     std::vector<spdlog::sink_ptr> sinks;
//     auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
//     console_sink->set_pattern("[%T] [%^%l%$] %v");
//     sinks.push_back(console_sink);

//     auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/mint_log.txt", true);
//     file_sink->set_pattern("[%Y-%m-%d %T] [%l] %v");
//     sinks.push_back(file_sink);

//     m_logger = std::make_shared<spdlog::logger>("MintLogger", begin(sinks), end(sinks));
//     spdlog::register_logger(m_logger);
//     m_logger->set_level(spdlog::level::trace);
//     m_logger->flush_on(spdlog::level::trace);
// }
// catch (const spdlog::spdlog_ex &ex)  {
//     throw std::runtime_error(std::string("Log initialization failed: ") + ex.what());
// }