#include "common.hpp"
#include "input.hpp"
#include "window.hpp"
#include "utility.hpp"
#include "shape_renderer.hpp"
#include "ui.hpp"
#include "sudoku.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/hash.hpp>

#include <format>
#include <print>
#include <initializer_list>
#include <string>
#include <optional>
#include <unordered_map>

enum class next_state
{
    main_menu,
    game,
    exit,
};

constexpr auto clear_colour = sudoku::from_hex(0x222f3e);

namespace sudoku {

auto hovered_cell(sudoku_board& board, const window& w) -> sudoku_cell*
{
    const auto mouse_pos = w.mouse_pos();
    
    const auto board_size = 0.9 * std::min(w.width(), w.height());
    const auto cell_size = board_size / board.size();

    auto top_left = glm::ivec2{w.width() / 2, w.height() / 2};
    top_left.x -= board_size / 2;
    top_left.y -= board_size / 2;

    const auto mouse_pos_sudoku_space = mouse_pos - top_left;

    if (0 <= mouse_pos_sudoku_space.x && mouse_pos_sudoku_space.x < board_size && 0 <= mouse_pos_sudoku_space.y && mouse_pos_sudoku_space.y < board_size) {
        const auto x = mouse_pos_sudoku_space.x / cell_size;
        const auto y = mouse_pos_sudoku_space.y / cell_size;
        return &board.at(x, y);
    }

    return nullptr;
}

}


auto scene_main_menu(sudoku::window& window) -> next_state
{
    using namespace sudoku;
    auto timer = sudoku::timer{};
    auto ui    = sudoku::ui_engine{};

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
        ui.text_box(sudoku::format_to(buf, "{}", timer.frame_rate()), {0, 0}, 120, 50, 3);
        ui.draw_frame(window.width(), window.height(), dt);
        window.end_frame();
    }

    return next_state::exit;
}

auto scene_game(sudoku::window& window) -> next_state
{
    using namespace sudoku;
    auto timer = sudoku::timer{};
    auto ui    = sudoku::ui_engine{};

#if 1
    auto board = make_board({
        "2..91.568",
        "...2541..",
        "1.....3..",
        "3....96..",
        "958.62..4",
        ".74.38.1.",
        ".81.4..26",
        "...7..8..",
        "..6891..3"
    });
#else
    auto board = make_board({
        "2..9",
        "...2",
        "1...",
        "3...",
    });
#endif

    while (window.is_running()) {
        const double dt = timer.on_update();
        window.begin_frame(clear_colour);

        for (const auto event : window.events()) {
            ui.on_event(event);

            if (auto e = event.get_if<keyboard_pressed_event>()) {
                if (auto cell = hovered_cell(board, window)) {
                    switch (e->key) {
                        case keyboard::num_1: cell->value = 1; break;
                        case keyboard::num_2: cell->value = 2; break;
                        case keyboard::num_3: cell->value = 3; break;
                        case keyboard::num_4: cell->value = 4; break;
                        case keyboard::num_5: cell->value = 5; break;
                        case keyboard::num_6: cell->value = 6; break;
                        case keyboard::num_7: cell->value = 7; break;
                        case keyboard::num_8: cell->value = 8; break;
                        case keyboard::num_9: cell->value = 9; break;
                    }
                }
            }
        }

        if (ui.button("Back", {0, 0}, 100, 50, 3)) {
            std::print("exiting!\n");
            return next_state::main_menu;
        }

        const auto board_size = 0.9 * std::min(window.width(), window.height());
        const auto cell_size = board_size / board.size();

        auto top_left = glm::ivec2{window.width() / 2, window.height() / 2};
        top_left.x -= board_size / 2;
        top_left.y -= board_size / 2;

        for (int y = 0; y != board.size(); ++y) {
            for (int x = 0; x != board.size(); ++x) {
                auto cell_top_left = top_left;
                cell_top_left.x += x * cell_size;
                cell_top_left.y += y * cell_size;
                ui.cell(board.at(x, y), {x, y}, cell_top_left, cell_size, cell_size);
            }
        }

       // ui.box(top_left, board_size, board_size);

        ui.draw_frame(window.width(), window.height(), dt);
        window.end_frame();
    }

    return next_state::exit;
}

auto main() -> int
{
    using namespace sudoku;

    auto window = sudoku::window{"The Way of Sudoku", 1280, 720};
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
                return 0;
            } break;
        }
    }
    
    return 0;
}