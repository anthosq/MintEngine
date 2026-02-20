#pragma once
#include "log_system.h"

#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

namespace Mint {
    struct AssimpLogStream : public Assimp::LogStream {
        static void Initialize() {
            if (Assimp::DefaultLogger::isNullLogger()) {
                Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
                Assimp::DefaultLogger::get()->attachStream(new AssimpLogStream, Assimp::Logger::Debugging | Assimp::Logger::Info | Assimp::Logger::Warn | Assimp::Logger::Err);
            }
        }

        virtual void write(const char *message) override
        {
            std::string msg(message);
            if (!msg.empty() && msg.back() == '\n') {
                msg.pop_back(); // Remove the trailing newline character
            }
            if (strncmp(message, "Error", 5) == 0) {
                LOG_ERROR(std::format("[Assimp] {0}", msg));
            } else if (strncmp(message, "Warn", 4) == 0) {
                LOG_WARN(std::format("[Assimp] {0}", msg));
            } else if (strncmp(message, "Info", 4) == 0) {
                LOG_INFO(std::format("[Assimp] {0}", msg));
            } else {
                LOG_DEBUG(std::format("[Assimp] {0}", msg));
            }
        }
    };
}