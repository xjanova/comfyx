#include "Config.h"
#include "PortablePaths.h"
#include <fstream>
#include <iostream>

namespace ComfyX {

Config& Config::instance() {
    static Config inst;
    return inst;
}

void Config::load() {
    auto path = PortablePaths::instance().configFile();
    if (!std::filesystem::exists(path)) {
        save(); // Create default config
        return;
    }

    try {
        std::ifstream file(path);
        nlohmann::json j;
        file >> j;
        m_config = j.get<AppConfig>();
    } catch (const std::exception& e) {
        std::cerr << "[Config] Failed to load config: " << e.what() << std::endl;
        m_config = AppConfig{}; // Reset to defaults
    }
}

void Config::save() {
    auto path = PortablePaths::instance().configFile();
    try {
        nlohmann::json j = m_config;
        std::ofstream file(path);
        file << j.dump(2);
    } catch (const std::exception& e) {
        std::cerr << "[Config] Failed to save config: " << e.what() << std::endl;
    }
}

} // namespace ComfyX
