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
#include <unordered_set>

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

auto check_solution(const sudoku_board& board) -> bool
{
    // check rows
    for (i32 row = 0; row != board.size(); ++row) {
        std::unordered_set<i32> seen; 
        for (i32 col = 0; col != board.size(); ++col) {
            const auto val = board.at(row, col).value;
            if (!val.has_value()) return false;
            seen.insert(*val);
        }
        if (seen.size() != board.size()) return false; // duplicate values in the row
    }

    // check columns
    for (i32 col = 0; col != board.size(); ++col) {
        std::unordered_set<i32> seen; 
        for (i32 row = 0; row != board.size(); ++row) {
            const auto val = board.at(row, col).value;
            seen.insert(*val);
        }
        if (seen.size() != board.size()) return false; // duplicate values in the row
    }

    // check regions
    std::unordered_map<i32, std::unordered_set<i32>> regions;
    for (i32 row = 0; row != board.size(); ++row) {
        for (i32 col = 0; col != board.size(); ++col) {
            if (board.at(row, col).region.has_value()) {
                regions[*board.at(row,col).region].insert(*board.at(row, col).value);
            }
        }
    }
    for (const auto& [region, seen] : regions) {
        if (seen.size() != board.size()) return false;
    }

    return true;
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
        ui.end_frame(dt);
        
        ui.draw_frame(window.width(), window.height());
        window.end_frame();
    }

    return next_state::exit;
}

auto scene_game(sudoku::window& window) -> next_state
{
    using namespace sudoku;
    auto timer = sudoku::timer{};
    auto ui    = sudoku::ui_engine{};
    auto shapes = sudoku::shape_renderer{};

#if 0
    auto board = make_board(
        {
            "2..91.568",
            "...2541..",
            "1.....3..",
            "3....96..",
            "958.62..4",
            ".74.38.1.",
            ".81.4..26",
            "...7..8..",
            "..6891..3",
        }, {
            "111222333",
            "111222333",
            "111222333",
            "444555666",
            "444555666",
            "444555666",
            "777888999",
            "777888999",
            "777888999",
        }
    );
#else
    auto board = make_board(
        {
            "..57341",
            "54..217",
            "3714..6",
            "1.6375.",
            "..2..7.",
            "4.75.62",
            "7..2.35",
        }, {
            "1122223",
            "1112223",
            "4115533",
            "4455533",
            "4455663",
            "4777666",
            "4777766",
        }
    );
#endif

    while (window.is_running()) {
        const double dt = timer.on_update();
        window.begin_frame(clear_colour);

        for (const auto event : window.events()) {
            ui.on_event(event);

            if (auto e = event.get_if<keyboard_pressed_event>()) {
                if (auto cell = hovered_cell(board, window); cell && !cell->fixed) {
                    std::optional<i32> value = {};
                    switch (e->key) {
                        case keyboard::num_1: value = 1; break;
                        case keyboard::num_2: value = 2; break;
                        case keyboard::num_3: value = 3; break;
                        case keyboard::num_4: value = 4; break;
                        case keyboard::num_5: value = 5; break;
                        case keyboard::num_6: value = 6; break;
                        case keyboard::num_7: value = 7; break;
                        case keyboard::num_8: value = 8; break;
                        case keyboard::num_9: value = 9; break;
                        case keyboard::backspace: value = -1; break;
                    }
                    if (value) {
                        if (*value == -1) {
                            cell->value = {};
                        } else if (*value <= board.size()) {
                            cell->value = *value;
                        }
                    }
                }
            }
        }

        if (ui.button("Back", {0, 0}, 200, 50, 3)) {
            std::print("exiting!\n");
            return next_state::main_menu;
        }

        if (ui.button("Check Solution", {0, 55}, 200, 50, 3)) {
            const auto success =  check_solution(board);
            if (success) {
                std::print("solved!\n");
            } else {
                std::print("bad!\n");
            }
        }

        const auto board_size = 0.9f * std::min(window.width(), window.height());
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

        // draw boundary
        const auto tl = glm::vec2{top_left};
        const auto tr = glm::vec2{top_left} + glm::vec2{board_size, 0};
        const auto bl = glm::vec2{top_left} + glm::vec2{0, board_size};
        const auto br = glm::vec2{top_left} + glm::vec2{board_size, board_size};

        for (i32 i = 1; i != board.size(); ++i) {
            const auto offset = glm::vec2{0, i * cell_size};
            shapes.draw_line(tl + offset, tr + offset, from_hex(0x7f8c8d), 0.5f);
        }
        for (i32 i = 1; i != board.size(); ++i) {
            const auto offset = glm::vec2{i * cell_size, 0};
            shapes.draw_line(tl + offset, bl + offset, from_hex(0x7f8c8d), 0.5f);
        }

        shapes.draw_line(tl, tr, from_hex(0xecf0f1), 2.5f);
        shapes.draw_line(tr, br, from_hex(0xecf0f1), 2.5f);
        shapes.draw_line(br, bl, from_hex(0xecf0f1), 2.5f);
        shapes.draw_line(bl, tl, from_hex(0xecf0f1), 2.5f);

        // draw regions
        for (i32 x = 0; x != board.size(); ++x) {
            for (i32 y = 0; y != board.size(); ++y) {
                if (x + 1 < board.size() && board.at(x, y).region != board.at(x + 1, y).region) {
                    const auto a = tl + cell_size * glm::vec2{x + 1, y};
                    const auto b = tl + cell_size * glm::vec2{x + 1, y + 1};
                    shapes.draw_line(a, b, from_hex(0xecf0f1), 2.5f);
                }

                if (y + 1 < board.size() && board.at(x, y).region != board.at(x, y + 1).region) {
                    const auto a = tl + cell_size * glm::vec2{x,     y + 1};
                    const auto b = tl + cell_size * glm::vec2{x + 1, y + 1};
                    shapes.draw_line(a, b, from_hex(0xecf0f1), 2.5f);
                }
            }
        }

        ui.end_frame(dt);

        ui.draw_frame(window.width(), window.height());
        shapes.draw_frame(window.width(), window.height());
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