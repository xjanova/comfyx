#pragma once
#include <string>

namespace ComfyX {

class MachineFingerprint {
public:
    // Get unique machine ID (stable across reboots)
    static std::string getMachineId();

    // Get hardware fingerprint hash
    static std::string getFingerprint();

    // Get OS information
    static std::string getOSInfo();

private:
    static std::string sha256(const std::string& input);
    static std::string getComputerName();
    static std::string getVolumeSerial();
    static std::string getCPUId();
};

} // namespace ComfyX
