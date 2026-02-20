#include "MachineFingerprint.h"

#ifdef _WIN32
#include <windows.h>
#include <intrin.h>
#include <wincrypt.h>
#pragma comment(lib, "advapi32.lib")
#endif

#include <sstream>
#include <iomanip>
#include <array>

namespace ComfyX {

std::string MachineFingerprint::getMachineId() {
    // Combine multiple identifiers for a stable machine ID
    std::string raw = getComputerName() + "|" + getVolumeSerial() + "|" + getCPUId();
    return sha256(raw).substr(0, 32); // First 32 chars of hash
}

std::string MachineFingerprint::getFingerprint() {
    // More detailed fingerprint
    std::string raw = getMachineId() + "|" + getOSInfo();
    return sha256(raw);
}

std::string MachineFingerprint::getOSInfo() {
#ifdef _WIN32
    // Use RtlGetVersion via NTDLL for accurate version (GetVersionEx lies on Win 8.1+)
    using RtlGetVersionFunc = LONG(WINAPI*)(PRTL_OSVERSIONINFOW);
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll) {
        auto RtlGetVersion = reinterpret_cast<RtlGetVersionFunc>(
            GetProcAddress(ntdll, "RtlGetVersion"));
        if (RtlGetVersion) {
            RTL_OSVERSIONINFOW info = {};
            info.dwOSVersionInfoSize = sizeof(info);
            if (RtlGetVersion(&info) == 0) {
                std::stringstream ss;
                ss << "Windows " << info.dwMajorVersion << "." << info.dwMinorVersion
                   << " Build " << info.dwBuildNumber;
                return ss.str();
            }
        }
    }
    return "Windows (version unknown)";
#else
    return "Unknown OS";
#endif
}

std::string MachineFingerprint::getComputerName() {
#ifdef _WIN32
    char name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(name);
    if (GetComputerNameA(name, &size)) {
        return std::string(name, size);
    }
#endif
    return "unknown";
}

std::string MachineFingerprint::getVolumeSerial() {
#ifdef _WIN32
    DWORD serial = 0;
    if (GetVolumeInformationA("C:\\", nullptr, 0, &serial, nullptr, nullptr, nullptr, 0)) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0') << std::setw(8) << serial;
        return ss.str();
    }
#endif
    return "00000000";
}

std::string MachineFingerprint::getCPUId() {
#ifdef _WIN32
    std::array<int, 4> cpuInfo;
    __cpuid(cpuInfo.data(), 0);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int v : cpuInfo) {
        ss << std::setw(8) << v;
    }
    return ss.str();
#else
    return "0000000000000000";
#endif
}

// Helper: DJB2 fallback hash (non-cryptographic, for identification only)
static std::string djb2Hash(const std::string& input) {
    unsigned long h = 5381;
    for (char c : input) {
        h = ((h << 5) + h) + c;
    }
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << h;
    return ss.str();
}

std::string MachineFingerprint::sha256(const std::string& input) {
#ifdef _WIN32
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    BYTE hash[32];
    DWORD hashLen = 32;

    if (!CryptAcquireContextW(&hProv, nullptr, nullptr, PROV_RSA_AES, CRYPT_VERIFYCONTEXT)) {
        return djb2Hash(input);
    }

    if (!CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return djb2Hash(input);
    }

    if (!CryptHashData(hHash, (const BYTE*)input.c_str(), (DWORD)input.size(), 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return djb2Hash(input);
    }

    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return djb2Hash(input);
    }

    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (DWORD i = 0; i < hashLen; i++) {
        ss << std::setw(2) << (int)hash[i];
    }
    return ss.str();
#else
    // Fallback hash (not cryptographic, just for ID purposes)
    return djb2Hash(input);
#endif
}

} // namespace ComfyX
