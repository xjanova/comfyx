#pragma once
#include <imgui.h>

namespace ComfyX {

struct Palette {
    // Background layers (ultra-dark, like ZipCrackerUI)
    ImVec4 bgDeep;          // Deepest: #0a0a12
    ImVec4 bgPrimary;       // Primary surface: #12121f
    ImVec4 bgSecondary;     // Content area: #16162a
    ImVec4 bgElevated;      // Cards/elevated: #1a1a2e
    ImVec4 bgPanel;         // Panel inner: #06060c (semi-transparent panels)

    // Borders
    ImVec4 border;          // Standard border: #2a2a4a
    ImVec4 borderSubtle;    // Subtle border: #1a1a30
    ImVec4 borderGlow;      // Glow border (for hover): #3a3a5a

    // Neon accent colors
    ImVec4 accent;          // Primary accent (neon cyan): #00f5ff
    ImVec4 accentHover;     // Accent hover (brighter): #66f9ff
    ImVec4 accentMuted;     // Accent with low alpha: #00f5ff @ 12%
    ImVec4 accentPink;      // Neon pink: #ff00ff
    ImVec4 accentPurple;    // Neon purple: #bf00ff

    // Semantic colors
    ImVec4 success;         // Neon green: #00ff88
    ImVec4 error;           // Neon red: #ff3366
    ImVec4 warning;         // Neon orange: #ff6b35

    // Text
    ImVec4 textPrimary;     // Primary text: #e0e0e0
    ImVec4 textSecondary;   // Secondary text: #808090
    ImVec4 textMuted;       // Muted/disabled: #4a4a5a
};

class Theme {
public:
    enum class Style {
        Midnight,   // Legacy dark blue
        Light,      // Light theme
        Modern      // Neon dark theme (inspired by X-Repass)
    };

    static void apply(Style style);
    static void applyMidnight();
    static void applyLight();
    static void applyModern();

    static const Palette& getPalette();

    // Helper: convert hex color to ImVec4
    static ImVec4 hex(unsigned int hexColor, float alpha = 1.0f);

    // Node graph colors
    struct NodeColors {
        static constexpr unsigned int Sampler      = 0xFF'FF'9E'4A;
        static constexpr unsigned int Loader       = 0xFF'7F'FF'4A;
        static constexpr unsigned int Conditioning = 0xFF'4A'9F'FF;
        static constexpr unsigned int Output       = 0xFF'6A'4A'FF;
        static constexpr unsigned int Latent       = 0xFF'E0'9F'FF;
        static constexpr unsigned int Image        = 0xFF'4A'E0'FF;
        static constexpr unsigned int Default      = 0xFF'A0'A0'A0;
    };

    static unsigned int getNodeColor(const char* nodeType);
};

} // namespace ComfyX
