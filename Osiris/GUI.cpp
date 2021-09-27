#include <algorithm>
#include <array>
#include <fstream>
#include <iterator>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <ShlObj.h>
#include <Windows.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include "imguiCustom.h"

#include "GUI.h"
#include "Config.h"
#include "ConfigStructs.h"
#include "InventoryChanger/InventoryChanger.h"
#include "Helpers.h"
#include "Interfaces.h"
#include "SDK/InputSystem.h"
#include <Hacks/Misc.h>
#include "Gilroy-Medium.h"
#include "Icons.h"

constexpr auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

ImFont* g_TabsFont;

void SidebarRender() {
    auto& s = ImGui::GetStyle();
    auto d = ImGui::GetWindowDrawList();
    auto p = ImGui::GetWindowPos();
    d->AddRectFilled(p, ImVec2(p.x + 950, p.y + 700), ImGui::GetColorU32(ImGuiCol_WindowBg), s.WindowRounding);
    d->AddRectFilled(p, ImVec2(p.x + 200, p.y + 700), ImGui::GetColorU32(ImGuiCol_ChildBg), s.WindowRounding, 0x5);

    ImGui::BeginGroup();
    {
        d->AddText(g_TabsFont, 16.f, ImVec2(p.x + (190 - ImGui::CalcTextSize("CSChanger.ru").x) / 2, p.y + 20), ImGui::GetColorU32(ImGuiCol_Text), "CSChanger.ru");
    }
    ImGui::EndGroup();
}

static ImFont* addFontFromVFONT(const std::string& path, float size, const ImWchar* glyphRanges, bool merge) noexcept
{
    auto file = Helpers::loadBinaryFile(path);
    if (!Helpers::decodeVFONT(file))
        return nullptr;

    ImFontConfig cfg;
    cfg.FontData = file.data();
    cfg.FontDataSize = file.size();
    cfg.FontDataOwnedByAtlas = false;
    cfg.MergeMode = merge;
    cfg.GlyphRanges = glyphRanges;
    cfg.SizePixels = size;

    return ImGui::GetIO().Fonts->AddFont(&cfg);
}

GUI::GUI() noexcept
{
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();
    s.FrameRounding = 0;
    s.WindowRounding = 0;
    s.ChildRounding = 0;
    s.ScrollbarRounding = 5.f;
    s.PopupRounding = 0;
    s.ScrollbarSize = 4.f;
    s.FramePadding = ImVec2(3, 2);

    auto c = s.Colors;
    c[ImGuiCol_WindowBg] = ImColor(33, 36, 41);
    c[ImGuiCol_FrameBg] = ImColor(38, 41, 46);
    c[ImGuiCol_FrameBgHovered] = ImColor(255, 90, 90, 200);
    c[ImGuiCol_FrameBgActive] = ImColor(255, 90, 90);
    c[ImGuiCol_Separator] = ImColor(255, 90, 90);
    c[ImGuiCol_PopupBg] = ImColor(33, 36, 41);
    c[ImGuiCol_ScrollbarBg] = ImColor(38, 41, 46);
    c[ImGuiCol_ScrollbarGrab] = ImColor(78, 124, 255);
    c[ImGuiCol_ScrollbarGrabActive] = ImColor(78, 124, 255);
    c[ImGuiCol_ScrollbarGrabHovered] = ImColor(78, 124, 255);
    c[ImGuiCol_Border] = ImColor();
    c[ImGuiCol_ChildBg] = ImColor(43, 46, 51);
    c[ImGuiCol_Header] = ImColor(38, 41, 46);
    c[ImGuiCol_HeaderHovered] = ImColor(78, 124, 255, 200);
    c[ImGuiCol_HeaderActive] = ImColor(78, 124, 255);
    c[ImGuiCol_Button] = ImColor(38, 41, 46);
    c[ImGuiCol_ButtonHovered] = ImColor(78, 124, 255, 200);
    c[ImGuiCol_ButtonActive] = ImColor(78, 124, 255);
    c[ImGuiCol_Text] = ImColor(255, 255, 255);
    c[ImGuiCol_TextDisabled] = ImColor(95, 98, 103);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

    ImFontConfig cfg;
    cfg.SizePixels = 15.0f;

    if (PWSTR pathToFonts; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Fonts, 0, nullptr, &pathToFonts))) {
        const std::filesystem::path path{ pathToFonts };
        CoTaskMemFree(pathToFonts);

        fonts.normal15px = io.Fonts->AddFontFromFileTTF((path / "tahoma.ttf").string().c_str(), 15.0f, &cfg, Helpers::getFontGlyphRanges());
        if (!fonts.normal15px)
            io.Fonts->AddFontDefault(&cfg);

        cfg.MergeMode = true;
        static constexpr ImWchar symbol[]{
            0x2605, 0x2605, // ★
            0
        };
        io.Fonts->AddFontFromFileTTF((path / "seguisym.ttf").string().c_str(), 15.0f, &cfg, symbol);
        cfg.MergeMode = false;
    }
    g_TabsFont = io.Fonts->AddFontFromMemoryTTF(GilroyMedium, sizeof(GilroyMedium), 14.f, NULL, io.Fonts->GetGlyphRangesCyrillic());

    if (!fonts.normal15px)
        io.Fonts->AddFontDefault(&cfg);

    addFontFromVFONT("csgo/panorama/fonts/notosanskr-regular.vfont", 15.0f, io.Fonts->GetGlyphRangesKorean(), true);
    addFontFromVFONT("csgo/panorama/fonts/notosanssc-regular.vfont", 17.0f, io.Fonts->GetGlyphRangesChineseFull(), true);
}

void GUI::render() noexcept
{
    ImGui::SetNextWindowSize(ImVec2(950, 700));
    ImGui::Begin("CSChanger", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
    {
        SidebarRender();
        InventoryChanger::drawGUI(false);
    }
    ImGui::End();
}

void GUI::updateColors() const noexcept
{
    
}

void GUI::handleToggle() noexcept
{
    if (Misc::isMenuKeyPressed()) {
        open = !open;

        if (!open) {
            config->save(0);
            interfaces->inputSystem->resetInputState();
        }

        ImGui::GetIO().MouseDrawCursor = gui->open;
    }
}

static void menuBarItem(const char* name, bool& enabled) noexcept
{
    if (ImGui::MenuItem(name)) {
        enabled = true;
        ImGui::SetWindowFocus(name);
        ImGui::SetWindowPos(name, { 100.0f, 100.0f });
    }
}

void GUI::renderMenuBar() noexcept
{
    if (ImGui::BeginMainMenuBar()) {
        InventoryChanger::menuBarItem();
        menuBarItem("Config", window.config);
        ImGui::EndMainMenuBar();   
    }
}

void GUI::renderStyleWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.style)
            return;
        ImGui::SetNextWindowSize({ 0.0f, 0.0f });
        ImGui::Begin("Style", &window.style, windowFlags);
    }

    ImGui::PushItemWidth(150.0f);
    if (ImGui::Combo("Menu style", &config->style.menuStyle, "Classic\0One window\0"))
        window = { };
    if (ImGui::Combo("Menu colors", &config->style.menuColors, "Dark\0Light\0Classic\0Custom\0"))
        updateColors();
    ImGui::PopItemWidth();

    if (config->style.menuColors == 3) {
        ImGuiStyle& style = ImGui::GetStyle();
        for (int i = 0; i < ImGuiCol_COUNT; i++) {
            if (i && i & 3) ImGui::SameLine(220.0f * (i & 3));

            ImGuiCustom::colorPicker(ImGui::GetStyleColorName(i), (float*)&style.Colors[i], &style.Colors[i].w);
        }
    }

    if (!contentOnly)
        ImGui::End();
}

void GUI::renderConfigWindow(bool contentOnly) noexcept
{
    if (!contentOnly) {
        if (!window.config)
            return;
        ImGui::SetNextWindowSize({ 320.0f, 0.0f });
        if (!ImGui::Begin("Config", &window.config, windowFlags)) {
            ImGui::End();
            return;
        }
    }

    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnOffset(1, 170.0f);

    static bool incrementalLoad = false;
    ImGui::Checkbox("Incremental Load", &incrementalLoad);

    ImGui::PushItemWidth(160.0f);

    auto& configItems = config->getConfigs();
    static int currentConfig = -1;

    static std::u8string buffer;

    timeToNextConfigRefresh -= ImGui::GetIO().DeltaTime;
    if (timeToNextConfigRefresh <= 0.0f) {
        config->listConfigs();
        if (const auto it = std::find(configItems.begin(), configItems.end(), buffer); it != configItems.end())
            currentConfig = std::distance(configItems.begin(), it);
        timeToNextConfigRefresh = 0.1f;
    }

    if (static_cast<std::size_t>(currentConfig) >= configItems.size())
        currentConfig = -1;

    if (ImGui::ListBox("", &currentConfig, [](void* data, int idx, const char** out_text) {
        auto& vector = *static_cast<std::vector<std::u8string>*>(data);
        *out_text = (const char*)vector[idx].c_str();
        return true;
        }, &configItems, configItems.size(), 5) && currentConfig != -1)
            buffer = configItems[currentConfig];

        ImGui::PushID(0);
        if (ImGui::InputTextWithHint("", "config name", &buffer, ImGuiInputTextFlags_EnterReturnsTrue)) {
            if (currentConfig != -1)
                config->rename(currentConfig, buffer);
        }
        ImGui::PopID();
        ImGui::NextColumn();

        ImGui::PushItemWidth(100.0f);

        if (ImGui::Button("Open config directory"))
            config->openConfigDir();

        if (ImGui::Button("Create config", { 100.0f, 25.0f }))
            config->add(buffer.c_str());

        if (ImGui::Button("Reset config", { 100.0f, 25.0f }))
            ImGui::OpenPopup("Config to reset");

        if (ImGui::BeginPopup("Config to reset")) {
            static constexpr const char* names[]{ "Whole", "Aimbot", "Triggerbot", "Backtrack", "Anti aim", "Glow", "Chams", "ESP", "Visuals", "Inventory Changer", "Sound", "Style", "Misc" };
            for (int i = 0; i < IM_ARRAYSIZE(names); i++) {
                if (i == 1) ImGui::Separator();

                if (ImGui::Selectable(names[i])) {
                    switch (i) {
                    case 0: config->reset(); updateColors(); InventoryChanger::scheduleHudUpdate(); break;
                    case 9: InventoryChanger::resetConfig(); InventoryChanger::scheduleHudUpdate(); break;
                    case 11: config->style = { }; updateColors(); break;
                    }
                }
            }
            ImGui::EndPopup();
        }
        if (currentConfig != -1) {
            if (ImGui::Button("Load selected", { 100.0f, 25.0f })) {
                config->load(currentConfig, incrementalLoad);
                updateColors();
                InventoryChanger::scheduleHudUpdate();
            }
            if (ImGui::Button("Save selected", { 100.0f, 25.0f }))
                config->save(currentConfig);
            if (ImGui::Button("Delete selected", { 100.0f, 25.0f })) { 

                config->remove(currentConfig);

                if (static_cast<std::size_t>(currentConfig) < configItems.size())
                    buffer = configItems[currentConfig];
                else
                    buffer.clear();
            }
        }
        ImGui::Columns(1);
        if (!contentOnly)
            ImGui::End();
}

void GUI::renderGuiStyle2() noexcept
{
    ImGui::Begin("Osiris", nullptr, windowFlags | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);

    if (ImGui::BeginTabBar("TabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll | ImGuiTabBarFlags_NoTooltip)) {
        InventoryChanger::tabItem();

        ImGui::EndTabBar();
    }

    ImGui::End();
}
