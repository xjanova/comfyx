#include "Theme.h"
#include <imgui.h>
#include <cstring>

namespace ComfyX {

void Theme::apply(Style style) {
    switch (style) {
        case Style::Midnight: applyMidnight(); break;
        case Style::Light:    applyLight(); break;
    }
}

void Theme::applyMidnight() {
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Window
    style.WindowRounding    = 6.0f;
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.PopupRounding     = 4.0f;
    style.ChildRounding     = 4.0f;

    style.WindowPadding     = ImVec2(10, 10);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ItemInnerSpacing  = ImVec2(6, 4);
    style.ScrollbarSize     = 12.0f;
    style.GrabMinSize       = 8.0f;
    style.WindowBorderSize  = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;

    // Midnight Studio color scheme
    ImVec4 bg_dark     = ImVec4(0.10f, 0.10f, 0.18f, 1.00f); // #1a1a2e
    ImVec4 bg_medium   = ImVec4(0.09f, 0.13f, 0.24f, 1.00f); // #16213e
    ImVec4 accent_blue = ImVec4(0.06f, 0.20f, 0.38f, 1.00f); // #0f3460
    ImVec4 accent_red  = ImVec4(0.91f, 0.27f, 0.38f, 1.00f); // #e94560
    ImVec4 text_main   = ImVec4(0.92f, 0.92f, 0.92f, 1.00f); // #eaeaea
    ImVec4 text_dim    = ImVec4(0.63f, 0.63f, 0.63f, 1.00f); // #a0a0a0

    colors[ImGuiCol_Text]                  = text_main;
    colors[ImGuiCol_TextDisabled]          = text_dim;
    colors[ImGuiCol_WindowBg]              = bg_dark;
    colors[ImGuiCol_ChildBg]               = ImVec4(0.08f, 0.08f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg]               = ImVec4(0.12f, 0.12f, 0.22f, 0.96f);
    colors[ImGuiCol_Border]                = ImVec4(0.18f, 0.22f, 0.35f, 0.60f);
    colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]               = ImVec4(0.12f, 0.14f, 0.25f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.15f, 0.18f, 0.32f, 1.00f);
    colors[ImGuiCol_FrameBgActive]         = accent_blue;
    colors[ImGuiCol_TitleBg]               = bg_medium;
    colors[ImGuiCol_TitleBgActive]         = ImVec4(0.12f, 0.16f, 0.30f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]      = bg_dark;
    colors[ImGuiCol_MenuBarBg]             = bg_medium;
    colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.08f, 0.15f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.20f, 0.24f, 0.38f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.26f, 0.30f, 0.46f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]   = accent_blue;
    colors[ImGuiCol_CheckMark]             = accent_red;
    colors[ImGuiCol_SliderGrab]            = ImVec4(0.29f, 0.62f, 1.00f, 0.80f);
    colors[ImGuiCol_SliderGrabActive]      = accent_red;
    colors[ImGuiCol_Button]                = ImVec4(0.14f, 0.18f, 0.32f, 1.00f);
    colors[ImGuiCol_ButtonHovered]         = ImVec4(0.18f, 0.24f, 0.42f, 1.00f);
    colors[ImGuiCol_ButtonActive]          = accent_blue;
    colors[ImGuiCol_Header]               = ImVec4(0.14f, 0.18f, 0.32f, 1.00f);
    colors[ImGuiCol_HeaderHovered]         = ImVec4(0.18f, 0.24f, 0.42f, 1.00f);
    colors[ImGuiCol_HeaderActive]          = accent_blue;
    colors[ImGuiCol_Separator]             = ImVec4(0.18f, 0.22f, 0.35f, 0.60f);
    colors[ImGuiCol_SeparatorHovered]      = accent_red;
    colors[ImGuiCol_SeparatorActive]       = accent_red;
    colors[ImGuiCol_ResizeGrip]            = ImVec4(0.18f, 0.22f, 0.35f, 0.40f);
    colors[ImGuiCol_ResizeGripHovered]     = accent_red;
    colors[ImGuiCol_ResizeGripActive]      = accent_red;
    colors[ImGuiCol_Tab]                   = ImVec4(0.10f, 0.14f, 0.26f, 1.00f);
    colors[ImGuiCol_TabHovered]            = ImVec4(0.18f, 0.24f, 0.42f, 1.00f);
    colors[ImGuiCol_TabSelected]           = accent_blue;
    colors[ImGuiCol_TabDimmed]             = ImVec4(0.08f, 0.10f, 0.18f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected]     = ImVec4(0.12f, 0.16f, 0.28f, 1.00f);
    colors[ImGuiCol_DockingPreview]        = ImVec4(accent_red.x, accent_red.y, accent_red.z, 0.60f);
    colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.06f, 0.06f, 0.10f, 1.00f);
    colors[ImGuiCol_PlotLines]             = ImVec4(0.29f, 0.62f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]      = accent_red;
    colors[ImGuiCol_PlotHistogram]         = ImVec4(0.29f, 0.62f, 1.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]  = accent_red;
    colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.12f, 0.14f, 0.25f, 1.00f);
    colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.18f, 0.22f, 0.35f, 1.00f);
    colors[ImGuiCol_TableBorderLight]      = ImVec4(0.15f, 0.18f, 0.28f, 1.00f);
    colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
    colors[ImGuiCol_TextSelectedBg]        = ImVec4(accent_red.x, accent_red.y, accent_red.z, 0.30f);
    colors[ImGuiCol_DragDropTarget]        = accent_red;
    colors[ImGuiCol_NavHighlight]          = accent_red;
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.12f);
    colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
    colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
}

void Theme::applyLight() {
    ImGui::StyleColorsLight();
    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding    = 6.0f;
    style.FrameRounding     = 4.0f;
    style.GrabRounding      = 4.0f;
    style.TabRounding       = 4.0f;
    style.ScrollbarRounding = 6.0f;
    style.PopupRounding     = 4.0f;
    style.ChildRounding     = 4.0f;

    style.WindowPadding     = ImVec2(10, 10);
    style.FramePadding      = ImVec2(8, 4);
    style.ItemSpacing       = ImVec2(8, 6);
    style.ItemInnerSpacing  = ImVec2(6, 4);
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
