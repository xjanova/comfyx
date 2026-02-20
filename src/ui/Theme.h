#pragma once

struct ImVec4;

namespace ComfyX {

class Theme {
public:
    enum class Style {
        Midnight,   // Dark blue theme
        Light       // Light theme
    };

    static void apply(Style style);
    static void applyMidnight();
    static void applyLight();

    // Node graph colors
    struct NodeColors {
        static constexpr unsigned int Sampler      = 0xFF'FF'9E'4A;  // Blue
        static constexpr unsigned int Loader       = 0xFF'7F'FF'4A;  // Green
        static constexpr unsigned int Conditioning = 0xFF'4A'9F'FF;  // Orange
        static constexpr unsigned int Output       = 0xFF'6A'4A'FF;  // Red
        static constexpr unsigned int Latent       = 0xFF'E0'9F'FF;  // Purple
        static constexpr unsigned int Image        = 0xFF'4A'E0'FF;  // Yellow
        static constexpr unsigned int Default      = 0xFF'A0'A0'A0;  // Gray
    };

    static unsigned int getNodeColor(const char* nodeType);
};

} // namespace ComfyX
