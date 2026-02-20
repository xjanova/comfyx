#pragma once
#include <string>
#include <iostream>
#include <mutex>

namespace ComfyX {

class Logger {
public:
    enum class Level { Debug, Info, Warning, Error };

    static void log(Level level, const std::string& tag, const std::string& message) {
        static std::mutex mutex;
        std::lock_guard<std::mutex> lock(mutex);

        const char* levelStr = "";
        switch (level) {
            case Level::Debug:   levelStr = "DEBUG"; break;
            case Level::Info:    levelStr = "INFO"; break;
            case Level::Warning: levelStr = "WARN"; break;
            case Level::Error:   levelStr = "ERROR"; break;
        }

        std::cout << "[" << levelStr << "] [" << tag << "] " << message << std::endl;
    }

    static void debug(const std::string& tag, const std::string& msg) { log(Level::Debug, tag, msg); }
    static void info(const std::string& tag, const std::string& msg) { log(Level::Info, tag, msg); }
    static void warn(const std::string& tag, const std::string& msg) { log(Level::Warning, tag, msg); }
    static void error(const std::string& tag, const std::string& msg) { log(Level::Error, tag, msg); }
};

} // namespace ComfyX
