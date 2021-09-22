#pragma once

#include <memory>

struct ImFont;

class GUI {
public:
    GUI() noexcept;
    void render() noexcept;
    void handleToggle() noexcept;
    [[nodiscard]] bool isOpen() const noexcept { return open; }
private:
    bool open = true;

    void updateColors() const noexcept;
    void renderMenuBar() noexcept;
    void renderStyleWindow(bool contentOnly = false) noexcept;
    void renderConfigWindow(bool contentOnly = false) noexcept;
    void renderGuiStyle2() noexcept;

    struct {
        bool style = false;
        bool config = false;
    } window;

    struct {
        ImFont* normal15px = nullptr;
    } fonts;

    float timeToNextConfigRefresh = 0.1f;
};

inline std::unique_ptr<GUI> gui;
