#include "common.hpp"
#include "input.hpp"
#include "window.hpp"
#include "utility.hpp"
#include "shape_renderer.hpp"
#include "ui.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

#include <format>
#include <print>

enum class next_state
{
    main_menu,
    game,
    exit,
};

constexpr auto clear_colour = sand::from_hex(0x222f3e);

auto scene_main_menu(sand::window& window) -> next_state
{
    using namespace sand;
    auto timer = sand::timer{};
    auto ui    = sand::ui_engine{};

    while (window.is_running()) {
        const double dt = timer.on_update();
        window.begin_frame(clear_colour);

        for (const auto event : window.events()) {
            ui.on_event(event);
        }
        
        const auto scale = 3.0f;
        const auto button_width = 200;
        const auto button_height = 50;
        const auto button_left = (window.width() - button_width) / 2;

        if (ui.button("Start Game", {button_left, 100}, button_width, button_height, scale)) {
            std::print("starting game!\n");
            return next_state::game;
        }

        if (ui.button("Exit", {button_left, 160}, button_width, button_height, scale)) {
            std::print("exiting!\n");
            return next_state::exit;
        }

        if (ui.button("Enable Vsync", {10, 50}, button_width, button_height, scale)) {
            window.enable_vsync(true);
        }
        if (ui.button("Disable Vsync", {10, 110}, button_width, button_height, scale)) {
            window.enable_vsync(false);
        }

        const auto para_left = 100;
        const auto para_top = 300;
        ui.text("Lorem ipsum dolor sit amet, consectetur adipiscing elit,", {para_left, para_top}, scale);
        ui.text("sed do eiusmod tempor incididunt ut labore et dolore magna", {para_left, para_top + 1 * 11 * scale}, scale);
        ui.text("aliqua. Ut enim ad minim veniam, quis nostrud exercitation", {para_left, para_top + 2 * 11 * scale}, scale);
        ui.text("ullamco laboris nisi ut aliquip ex ea commodo consequat.", {para_left, para_top + 3 * 11 * scale}, scale);
        ui.text("Duis aute irure dolor in reprehenderit in voluptate velit", {para_left, para_top + 4 * 11 * scale}, scale);
        ui.text("esse cillum dolore eu fugiat nulla pariatur. Excepteur", {para_left, para_top + 5 * 11 * scale}, scale);
        ui.text("sint occaecat cupidatat non proident, sunt in culpa", {para_left, para_top + 6 * 11 * scale}, scale);
        ui.text("qui officia deserunt mollit anim id est laborum.", {para_left, para_top + 7 * 11 * scale}, scale);
        ui.text("ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz", {para_left, para_top + 8 * 11 * scale}, scale);
        ui.text("0123456789 () {} [] ^ < > - _ = + ! ? : ; . , @ % $ / \\ \" ' # ~ & | `", {para_left, para_top + 9 * 11 * scale}, scale);

        std::array<char, 8> buf = {};
        ui.text_box(sand::format_to(buf, "{}", timer.frame_rate()), {0, 0}, 120, 50, 3);
        ui.draw_frame(window.width(), window.height(), dt);
        window.end_frame();
    }

    return next_state::exit;
}

auto scene_game(sand::window& window) -> next_state
{
    using namespace sand;
    auto timer = sand::timer{};
    auto ui    = sand::ui_engine{};

    while (window.is_running()) {
        const double dt = timer.on_update();
        window.begin_frame(clear_colour);

        for (const auto event : window.events()) {
            ui.on_event(event);
        }
        
        const auto scale = 3.0f;
        const auto button_width = 200;
        const auto button_height = 50;
        const auto button_left = (window.width() - button_width) / 2;

        if (ui.button("Back", {button_left, 160}, button_width, button_height, scale)) {
            std::print("exiting!\n");
            return next_state::main_menu;
        }

        ui.draw_frame(window.width(), window.height(), dt);
        window.end_frame();
    }

    return next_state::exit;
}

auto main() -> int
{
    using namespace sand;

    auto window = sand::window{"sandfall", 1280, 720};
    auto next   = next_state::main_menu;

    while (true) {
        switch (next) {
            case next_state::main_menu: {
                next = scene_main_menu(window);
            } break;
            case next_state::game: {
                next = scene_game(window);
            } break;
            case next_state::exit: {
                std::print("closing game\n");
                return 1;
            } break;
            default: {
                std::print("how did we get here\n");
                return 1; // TODO: Handle this better
            }
        }
    }
    
    return 0;
}