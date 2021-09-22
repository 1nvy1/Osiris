#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "ConfigStructs.h"
#include "InputUtil.h"

struct ImFont;

class Config {
public:
    Config() noexcept;
    void load(std::size_t, bool incremental) noexcept;
    void load(const char8_t* name, bool incremental) noexcept;
    void save(std::size_t) const noexcept;
    void add(const char8_t*) noexcept;
    void remove(std::size_t) noexcept;
    void rename(std::size_t, std::u8string_view newName) noexcept;
    void reset() noexcept;
    void listConfigs() noexcept;
    void createConfigDir() const noexcept;
    void openConfigDir() const noexcept;

    constexpr auto& getConfigs() noexcept
    {
        return configs;
    }

    struct Font {
        ImFont* tiny;
        ImFont* medium;
        ImFont* big;
    };
   
    struct Style {
        int menuStyle{ 0 };
        int menuColors{ 0 };
    } style;

    void scheduleFontLoad(const std::string& name) noexcept;
    bool loadScheduledFonts() noexcept;
    const auto& getSystemFonts() const noexcept { return systemFonts; }
    const auto& getFonts() const noexcept { return fonts; }
private:
    std::vector<std::string> scheduledFonts{ "Default" };
    std::vector<std::string> systemFonts{ "Default" };
    std::unordered_map<std::string, Font> fonts;
    std::filesystem::path path;
    std::vector<std::u8string> configs;
};

inline std::unique_ptr<Config> config;
