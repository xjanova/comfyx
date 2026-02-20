#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace ComfyX {

class FileUtils {
public:
    static std::string readFile(const std::filesystem::path& path);
    static bool writeFile(const std::filesystem::path& path, const std::string& content);
    static std::vector<uint8_t> readBinaryFile(const std::filesystem::path& path);
    static bool writeBinaryFile(const std::filesystem::path& path, const std::vector<uint8_t>& data);
};

} // namespace ComfyX
