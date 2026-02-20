#include "Theme.h"
#include <imgui.h>
#include <cstring>

namespace ComfyX {

static Palette s_palette;

const Palette& Theme::getPalette() {
    return s_palette;
}

ImVec4 Theme::hex(unsigned int hexColor, float alpha) {
    return ImVec4(
        ((hexColor >> 16) & 0xFF) / 255.0f,
        ((hexColor >> 8) & 0xFF) / 255.0f,
        (hexColor & 0xFF) / 255.0f,
        alpha);
}

void Theme::apply(Style style) {
    switch (style) {
        case Style::Midnight: applyMidnight(); break;
        case Style::Light:    applyLight(); break;
        case Style::Modern:   applyModern(); break;
        default:              applyModern(); break;
    }
}

void Theme::applyModern() {
    // ═══════════════════════════════════════════════════════════════════
    // NEON DARK THEME — inspired by X-Repass / ZipCrackerUI
    // Ultra-dark backgrounds with neon cyan/pink/green accents
    // ═══════════════════════════════════════════════════════════════════

    // Background layers (ultra-dark)
    s_palette.bgDeep       = hex(0x0a0a12);     // Deepest background
    s_palette.bgPrimary    = hex(0x12121f);     // Sidebar, top bar
    s_palette.bgSecondary  = hex(0x16162a);     // Content area
    s_palette.bgElevated   = hex(0x1a1a2e);     // Cards, elevated panels
    s_palette.bgPanel      = hex(0x06060c, 0.92f); // Semi-transparent panels

    // Borders
    s_palette.border       = hex(0x2a2a4a);
    s_palette.borderSubtle = hex(0x1a1a30);
    s_palette.borderGlow   = hex(0x3a3a5a);

    // Neon accents
    s_palette.accent       = hex(0x00f5ff);     // Neon Cyan
    s_palette.accentHover  = hex(0x66f9ff);     // Brighter cyan
    s_palette.accentMuted  = hex(0x00f5ff, 0.12f);
    s_palette.accentPink   = hex(0xff00ff);     // Neon Pink
    s_palette.accentPurple = hex(0xbf00ff);     // Neon Purple

    // Semantic colors
    s_palette.success      = hex(0x00ff88);     // Neon Green
    s_palette.error        = hex(0xff3366);     // Neon Red
    s_palette.warning      = hex(0xff6b35);     // Neon Orange

    // Text
    s_palette.textPrimary  = hex(0xe0e0e0);
    s_palette.textSecondary= hex(0x808090);
    s_palette.textMuted    = hex(0x4a4a5a);

    // ─── ImGui Style ───
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Geometry — generous rounding like ZipCrackerUI (CornerRadius 8-12)
    style.WindowRounding    = 12.0f;
    style.FrameRounding     = 8.0f;
    style.GrabRounding      = 8.0f;
    style.TabRounding       = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.PopupRounding     = 10.0f;
    style.ChildRounding     = 8.0f;

    // Spacing — comfortable like a modern creative app
    style.WindowPadding     = ImVec2(14, 14);
    style.FramePadding      = ImVec2(12, 7);
    style.ItemSpacing       = ImVec2(10, 8);
    style.ItemInnerSpacing  = ImVec2(8, 6);
    style.ScrollbarSize     = 10.0f;
    style.GrabMinSize       = 10.0f;
    style.IndentSpacing     = 20.0f;

    // Borders — minimal, let the neon glow speak
    style.WindowBorderSize  = 0.0f;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;
    style.ChildBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;
    style.SeparatorTextBorderSize = 1.0f;

    // ─── Colors ───
    const auto& p = s_palette;

    // Window backgrounds
    colors[ImGuiCol_Text]                  = p.textPrimary;
    colors[ImGuiCol_TextDisabled]          = p.textMuted;
    colors[ImGuiCol_WindowBg]              = p.bgDeep;
    colors[ImGuiCol_ChildBg]               = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_PopupBg]               = ImVec4(p.bgPrimary.x, p.bgPrimary.y, p.bgPrimary.z, 0.97f);
    colors[ImGuiCol_Border]                = ImVec4(p.border.x, p.border.y, p.border.z, 0.5f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);

    // Frame (input fields, etc) — darker interior
    colors[ImGuiCol_FrameBg]               = hex(0x0d0d1a);
    colors[ImGuiCol_FrameBgHovered]        = hex(0x202040);
    colors[ImGuiCol_FrameBgActive]         = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.25f);

    // Title bar
    colors[ImGuiCol_TitleBg]               = p.bgPrimary;
    colors[ImGuiCol_TitleBgActive]         = p.bgPrimary;
    colors[ImGuiCol_TitleBgCollapsed]      = p.bgDeep;
    colors[ImGuiCol_MenuBarBg]             = p.bgPrimary;

    // Scrollbar — subtle, dark
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0, 0, 0, 0.08f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(p.border.x, p.border.y, p.border.z, 0.5f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = hex(0x5a5a7a);
    colors[ImGuiCol_ScrollbarGrabActive]   = p.accent;

    // Check mark, slider — neon cyan
    colors[ImGuiCol_CheckMark]             = p.accent;
    colors[ImGuiCol_SliderGrab]            = p.accent;
    colors[ImGuiCol_SliderGrabActive]      = p.accentHover;

    // Buttons — dark with neon glow on hover
    colors[ImGuiCol_Button]                = hex(0x1e3a5f, 0.6f);
    colors[ImGuiCol_ButtonHovered]         = hex(0x2a4a6a, 0.8f);
    colors[ImGuiCol_ButtonActive]          = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.35f);

    // Headers
    colors[ImGuiCol_Header]               = hex(0x1a1a2e);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.18f);
    colors[ImGuiCol_HeaderActive]          = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.28f);

    // Separator
    colors[ImGuiCol_Separator]             = ImVec4(p.border.x, p.border.y, p.border.z, 0.4f);
    colors[ImGuiCol_SeparatorHovered]      = p.accent;
    colors[ImGuiCol_SeparatorActive]       = p.accent;

    // Resize grip
    colors[ImGuiCol_ResizeGrip]            = ImVec4(p.border.x, p.border.y, p.border.z, 0.2f);
    colors[ImGuiCol_ResizeGripHovered]     = p.accent;
    colors[ImGuiCol_ResizeGripActive]      = p.accentHover;

    // Tabs — dark, accent when selected
    colors[ImGuiCol_Tab]                   = hex(0x0d0d1a);
    colors[ImGuiCol_TabHovered]            = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.22f);
    colors[ImGuiCol_TabSelected]           = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.30f);
    colors[ImGuiCol_TabDimmed]             = p.bgDeep;
    colors[ImGuiCol_TabDimmedSelected]     = p.bgPrimary;

    // Docking
    colors[ImGuiCol_DockingPreview]        = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.5f);
    colors[ImGuiCol_DockingEmptyBg]        = p.bgDeep;

    // Plot
    colors[ImGuiCol_PlotLines]             = p.accent;
    colors[ImGuiCol_PlotLinesHovered]      = p.accentPink;
    colors[ImGuiCol_PlotHistogram]         = p.accent;
    colors[ImGuiCol_PlotHistogramHovered]  = p.accentPink;

    // Table
    colors[ImGuiCol_TableHeaderBg]         = hex(0x1a1a30);
    colors[ImGuiCol_TableBorderStrong]     = p.border;
    colors[ImGuiCol_TableBorderLight]      = p.borderSubtle;
    colors[ImGuiCol_TableRowBg]            = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1, 1, 1, 0.02f);

    // Selection and misc
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(p.accent.x, p.accent.y, p.accent.z, 0.25f);
    colors[ImGuiCol_DragDropTarget]        = p.accent;
    colors[ImGuiCol_NavHighlight]          = p.accent;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.12f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0, 0, 0, 0.6f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0, 0, 0, 0.65f);
}

void Theme::applyMidnight() {
    // Initialize palette for legacy theme
    s_palette.bgDeep       = hex(0x1a1a2e);
    s_palette.bgPrimary    = hex(0x172148);
    s_palette.bgSecondary  = hex(0x1e2440);
    s_palette.bgElevated   = hex(0x242e52);
    s_palette.bgPanel      = hex(0x0a0a14, 0.9f);
    s_palette.border       = ImVec4(0.18f, 0.22f, 0.35f, 0.6f);
    s_palette.borderSubtle = hex(0x262e48);
    s_palette.borderGlow   = hex(0x3a4a6a);
    s_palette.accent       = hex(0x1034a0);
    s_palette.accentHover  = hex(0x2e3e6a);
    s_palette.accentMuted  = hex(0x1034a0, 0.15f);
    s_palette.accentPink   = hex(0xff00ff);
    s_palette.accentPurple = hex(0xbf00ff);
    s_palette.success      = hex(0x4aff7d);
    s_palette.error        = hex(0xe84464);
    s_palette.warning      = hex(0xd29922);
    s_palette.textPrimary  = ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
    s_palette.textSecondary= ImVec4(0.63f, 0.63f, 0.63f, 1.0f);
    s_palette.textMuted    = ImVec4(0.40f, 0.40f, 0.40f, 1.0f);

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    style.WindowRounding    = 8.0f;
    style.FrameRounding     = 6.0f;
    style.GrabRounding      = 6.0f;
    style.TabRounding       = 6.0f;
    style.ScrollbarRounding = 8.0f;
    style.PopupRounding     = 6.0f;
    style.ChildRounding     = 6.0f;
    style.WindowPadding     = ImVec2(10, 10);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ItemInnerSpacing  = ImVec2(6, 4);
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 8.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;

    colors[ImGuiCol_Text]                  = s_palette.textPrimary;
    colors[ImGuiCol_TextDisabled]          = s_palette.textSecondary;
    colors[ImGuiCol_WindowBg]              = s_palette.bgDeep;
    colors[ImGuiCol_ChildBg]               = ImVec4(0.08f, 0.08f, 0.15f, 1.0f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.12f, 0.12f, 0.22f, 0.96f);
    colors[ImGuiCol_Border]                = s_palette.border;
    colors[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg]               = s_palette.bgSecondary;
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.15f, 0.18f, 0.32f, 1.0f);
    colors[ImGuiCol_FrameBgActive]         = s_palette.accent;
    colors[ImGuiCol_TitleBg]               = s_palette.bgPrimary;
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.12f, 0.16f, 0.30f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed]      = s_palette.bgDeep;
    colors[ImGuiCol_MenuBarBg]             = s_palette.bgPrimary;
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.08f, 0.15f, 0.6f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.20f, 0.24f, 0.38f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.26f, 0.30f, 0.46f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive]   = s_palette.accent;
    colors[ImGuiCol_CheckMark]             = s_palette.error;
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.29f, 0.62f, 1.0f, 0.8f);
    colors[ImGuiCol_SliderGrabActive]      = s_palette.error;
    colors[ImGuiCol_Button]                = s_palette.bgElevated;
    colors[ImGuiCol_ButtonHovered]         = s_palette.accentHover;
    colors[ImGuiCol_ButtonActive]          = s_palette.accent;
    colors[ImGuiCol_Header]               = s_palette.bgElevated;
    colors[ImGuiCol_HeaderHovered]         = s_palette.accentHover;
    colors[ImGuiCol_HeaderActive]          = s_palette.accent;
    colors[ImGuiCol_Separator]             = s_palette.border;
    colors[ImGuiCol_SeparatorHovered]      = s_palette.error;
    colors[ImGuiCol_SeparatorActive]       = s_palette.error;
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.18f, 0.22f, 0.35f, 0.4f);
    colors[ImGuiCol_ResizeGripHovered]     = s_palette.error;
    colors[ImGuiCol_ResizeGripActive]      = s_palette.error;
    colors[ImGuiCol_Tab]                   = ImVec4(0.10f, 0.14f, 0.26f, 1.0f);
    colors[ImGuiCol_TabHovered]            = s_palette.accentHover;
    colors[ImGuiCol_TabSelected]           = s_palette.accent;
    colors[ImGuiCol_TabDimmed]             = ImVec4(0.08f, 0.10f, 0.18f, 1.0f);
    colors[ImGuiCol_TabDimmedSelected]     = ImVec4(0.12f, 0.16f, 0.28f, 1.0f);
    colors[ImGuiCol_DockingPreview]        = ImVec4(s_palette.error.x, s_palette.error.y, s_palette.error.z, 0.6f);
    colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.06f, 0.06f, 0.10f, 1.0f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.29f, 0.62f, 1.0f, 1.0f);
    colors[ImGuiCol_PlotLinesHovered]      = s_palette.error;
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.29f, 0.62f, 1.0f, 1.0f);
    colors[ImGuiCol_PlotHistogramHovered]  = s_palette.error;
    colors[ImGuiCol_TableHeaderBg]         = s_palette.bgSecondary;
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.18f, 0.22f, 0.35f, 1.0f);
    colors[ImGuiCol_TableBorderLight]      = s_palette.borderSubtle;
    colors[ImGuiCol_TableRowBg]            = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1, 1, 1, 0.03f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(s_palette.error.x, s_palette.error.y, s_palette.error.z, 0.3f);
    colors[ImGuiCol_DragDropTarget]        = s_palette.error;
    colors[ImGuiCol_NavHighlight]          = s_palette.error;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.12f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0, 0, 0, 0.6f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0, 0, 0, 0.6f);
}

void Theme::applyLight() {
    s_palette.bgDeep       = ImVec4(0.95f, 0.95f, 0.95f, 1.0f);
    s_palette.bgPrimary    = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    s_palette.bgSecondary  = ImVec4(0.97f, 0.97f, 0.97f, 1.0f);
    s_palette.bgElevated   = ImVec4(0.92f, 0.92f, 0.92f, 1.0f);
    s_palette.bgPanel      = ImVec4(0.96f, 0.96f, 0.96f, 0.9f);
    s_palette.border       = ImVec4(0.80f, 0.80f, 0.80f, 1.0f);
    s_palette.borderSubtle = ImVec4(0.88f, 0.88f, 0.88f, 1.0f);
    s_palette.borderGlow   = ImVec4(0.70f, 0.70f, 0.70f, 1.0f);
    s_palette.accent       = ImVec4(0.10f, 0.46f, 0.82f, 1.0f);
    s_palette.accentHover  = ImVec4(0.15f, 0.53f, 0.90f, 1.0f);
    s_palette.accentMuted  = ImVec4(0.10f, 0.46f, 0.82f, 0.12f);
    s_palette.accentPink   = ImVec4(0.85f, 0.10f, 0.65f, 1.0f);
    s_palette.accentPurple = ImVec4(0.60f, 0.10f, 0.85f, 1.0f);
    s_palette.success      = ImVec4(0.18f, 0.64f, 0.27f, 1.0f);
    s_palette.error        = ImVec4(0.82f, 0.20f, 0.20f, 1.0f);
    s_palette.warning      = ImVec4(0.75f, 0.55f, 0.10f, 1.0f);
    s_palette.textPrimary  = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    s_palette.textSecondary= ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
    s_palette.textMuted    = ImVec4(0.65f, 0.65f, 0.65f, 1.0f);

    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding    = 10.0f;
    style.FrameRounding     = 8.0f;
    style.GrabRounding      = 8.0f;
    style.TabRounding       = 8.0f;
    style.ScrollbarRounding = 10.0f;
    style.PopupRounding     = 10.0f;
    style.ChildRounding     = 8.0f;
    style.WindowPadding     = ImVec2(14, 14);
    style.FramePadding      = ImVec2(12, 7);
    style.ItemSpacing       = ImVec2(10, 8);
    style.ItemInnerSpacing  = ImVec2(8, 6);
    style.WindowBorderSize  = 0.0f;
}

unsigned int Theme::getNodeColor(const char* nodeType) {
    if (!nodeType) return NodeColors::Default;

    if (strstr(nodeType, "Sampler") || strstr(nodeType, "KSampler"))
        return NodeColors::Sampler;
    if (strstr(nodeType, "Loader") || strstr(nodeType, "Load"))
        return NodeColors::Loader;
    if (strstr(nodeType, "CLIP") || strstr(nodeType, "Conditioning") || strstr(nodeType, "Prompt"))
        return NodeColors::Conditioning;
    if (strstr(nodeType, "Save") || strstr(nodeType, "Preview") || strstr(nodeType, "Output"))
        return NodeColors::Output;
    if (strstr(nodeType, "Latent") || strstr(nodeType, "VAE"))
        return NodeColors::Latent;
    if (strstr(nodeType, "Image"))
        return NodeColors::Image;

    return NodeColors::Default;
}

} // namespace ComfyX
