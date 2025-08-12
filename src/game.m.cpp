#include "common.hpp"
#include "input.hpp"
#include "window.hpp"
#include "utility.hpp"
#include "renderer.hpp"
#include "ui.hpp"
#include "sudoku.hpp"
#include "draw_board.hpp"

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

auto hovered_cell_pos(const sudoku_board& board, const window& w) -> std::optional<glm::ivec2>
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
        return glm::ivec2{x, y};
    }

    return {};
}

auto check_solution(const sudoku_board& board, time_point time) -> board_render_state
{
    auto empty_cells = empty_cells_rs{};
    empty_cells.time = time;

    // check for empty cells
    for (i32 row = 0; row != board.size(); ++row) {
        for (i32 col = 0; col != board.size(); ++col) {
            const auto val = board.at({row, col}).value;
            if (!val.has_value()) empty_cells.cells.insert(glm::ivec2{row, col});
        }
    }
    if (!empty_cells.cells.empty()) { // bad solution because the board isn't filled
        return empty_cells;
    }

    // check rows
    for (i32 row = 0; row != board.size(); ++row) {
        std::unordered_set<i32> seen; 
        for (i32 col = 0; col != board.size(); ++col) {
            const auto val = board.at({row, col}).value;
            seen.insert(*val);
        }
        if (seen.size() != board.size()) return constraint_faiure_rs{}; // duplicate values in the row
    }

    // check columns
    for (i32 col = 0; col != board.size(); ++col) {
        std::unordered_set<i32> seen; 
        for (i32 row = 0; row != board.size(); ++row) {
            const auto val = board.at({row, col}).value;
            seen.insert(*val);
        }
        if (seen.size() != board.size()) return constraint_faiure_rs{}; // duplicate values in the row
    }

    // check regions
    std::unordered_map<i32, std::unordered_set<i32>> regions;
    for (i32 row = 0; row != board.size(); ++row) {
        for (i32 col = 0; col != board.size(); ++col) {
            if (board.at({row, col}).region.has_value()) {
                regions[*board.at({row, col}).region].insert(*board.at({row, col}).value);
            }
        }
    }
    for (const auto& [region, seen] : regions) {
        if (seen.size() != board.size()) return constraint_faiure_rs{};
    }

    return solved_rs{ .time = time };
}

void flip(std::set<i32>& ints, i32 value)
{
    if (ints.contains(value)) {
        ints.erase(value);
    } else {
        ints.insert(value);
    }
}

}

auto scene_main_menu(sudoku::window& window) -> next_state
{
    using namespace sudoku;
    auto timer = sudoku::timer{};
    auto renderer = sudoku::renderer{};
    auto ui    = sudoku::ui_engine{&renderer};

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
        constexpr auto colour = from_hex(0xecf0f1);
        renderer.push_text("Lorem ipsum dolor sit amet, consectetur adipiscing elit,", {para_left, para_top}, scale, colour);
        renderer.push_text("sed do eiusmod tempor incididunt ut labore et dolore magna", {para_left, para_top + 1 * 11 * scale}, scale, colour);
        renderer.push_text("aliqua. Ut enim ad minim veniam, quis nostrud exercitation", {para_left, para_top + 2 * 11 * scale}, scale, colour);
        renderer.push_text("ullamco laboris nisi ut aliquip ex ea commodo consequat.", {para_left, para_top + 3 * 11 * scale}, scale, colour);
        renderer.push_text("Duis aute irure dolor in reprehenderit in voluptate velit", {para_left, para_top + 4 * 11 * scale}, scale, colour);
        renderer.push_text("esse cillum dolore eu fugiat nulla pariatur. Excepteur", {para_left, para_top + 5 * 11 * scale}, scale, colour);
        renderer.push_text("sint occaecat cupidatat non proident, sunt in culpa", {para_left, para_top + 6 * 11 * scale}, scale, colour);
        renderer.push_text("qui officia deserunt mollit anim id est laborum.", {para_left, para_top + 7 * 11 * scale}, scale, colour);
        renderer.push_text("ABCDEFGHIJKLMNOPQRSTUVWXYZ abcdefghijklmnopqrstuvwxyz", {para_left, para_top + 8 * 11 * scale}, scale, colour);
        renderer.push_text("0123456789 () {} [] ^ < > - _ = + ! ? : ; . , @ % $ / \\ \" ' # ~ & | `", {para_left, para_top + 9 * 11 * scale}, scale, colour);

        std::array<char, 8> buf = {};
        renderer.push_text_box(sudoku::format_to(buf, "{}", timer.frame_rate()), {0, 0}, 120, 50, 3, colour);
        ui.end_frame(dt);

        renderer.draw(window.width(), window.height());
        window.end_frame();
    }

    return next_state::exit;
}

auto scene_game(sudoku::window& window) -> next_state
{
    using namespace sudoku;
    auto timer    = sudoku::timer{};
    auto renderer = sudoku::renderer{};
    auto ui       = sudoku::ui_engine{&renderer};

    auto state = board_render_state{ normal_rs{} };

#define LEVEL 3
#if LEVEL == 0
    auto board = sudoku_board::make_board(
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
#elif LEVEL == 1
    auto board = sudoku_board::make_board(
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
#elif LEVEL == 2
    auto board = sudoku_board::make_board(
        {
            ".5...",
            "...3.",
            "5...1",
            ".4...",
            "...5."
        }, {
            "11112",
            "13442",
            "33442",
            "33422",
            "55555"
        }
    );
#elif LEVEL == 3
    auto board = sudoku_board::make_board(
        {
            "..12",
            "..3.",
            ".3..",
            "21.."
        }, {
            "1122",
            "1122",
            "3344",
            "3344"
        }
    );
#endif

    std::optional<bool> mouse_down = {};
    while (window.is_running()) {
        const double dt = timer.on_update();
        window.begin_frame(clear_colour);

        if (auto inner = std::get_if<empty_cells_rs>(&state)) {
            if (timer.now() - inner->time > 1s) {
                state = normal_rs{};
            }
        }

        for (const auto event : window.events()) {
            ui.on_event(event);
            if (std::holds_alternative<solved_rs>(state)) {
                continue; // Don't allow updating the board when it's solved
            }

            if (auto e = event.get_if<mouse_pressed_event>()) {
                const auto pos = hovered_cell_pos(board, window);
                if (pos.has_value() && board.valid(*pos)) {
                    if (e->button == mouse::left) {
                        if (e->mods & modifier::shift) {
                            board.toggle_selected(*pos);
                        } else {
                            board.unselect_all();
                            board.select(*pos, true);
                        }
                        mouse_down = std::as_const(board).at(*pos).selected;
                    }
                } else {
                    board.unselect_all();
                }
            }
            else if (auto e = event.get_if<mouse_released_event>()) {
                if (e->button == mouse::left) {
                    mouse_down = {};
                }
            }
            else if (auto e = event.get_if<mouse_moved_event>()) {
                if (mouse_down.has_value()) {
                    auto cell = hovered_cell_pos(board, window);
                    if (cell.has_value() && board.valid(*cell)) {
                        board.select(*cell, *mouse_down);
                    }
                }
            }
            else if (auto e = event.get_if<keyboard_pressed_event>()) {
                std::optional<i32> value = {};
                switch (e->key) {
                    case keyboard::Z: {
                        if (e->mods & modifier::ctrl) {
                            board.undo();
                        }
                    } break;
                    case keyboard::Y: {
                        if (e->mods & modifier::ctrl) {
                            board.redo();
                        }
                    } break;
                    case keyboard::backspace: {
                        board.clear_selected();
                    } break;
                    case keyboard::escape: {
                        board.unselect_all();
                    } break;
                    case keyboard::num_1: value = 1; break;
                    case keyboard::num_2: value = 2; break;
                    case keyboard::num_3: value = 3; break;
                    case keyboard::num_4: value = 4; break;
                    case keyboard::num_5: value = 5; break;
                    case keyboard::num_6: value = 6; break;
                    case keyboard::num_7: value = 7; break;
                    case keyboard::num_8: value = 8; break;
                    case keyboard::num_9: value = 9; break;
                }
                if (!value) continue; // keyboard input was not a digit
                if (*value > board.size()) continue; // not a digit in the grid

                if (e->mods & modifier::ctrl) {
                    board.set_centre_pencil_mark(*value);
                }
                else if (e->mods & modifier::shift) {
                    board.set_corner_pencil_mark(*value);
                }
                else {
                    board.set_digit(*value);
                }
            }
        }

        draw_board(renderer, {window.width(), window.height()}, board, state, timer.now());
        
        if (ui.button("Back", {0, 0}, 200, 50, 3)) {
            return next_state::main_menu;
        }

        if (ui.button("Check Solution", {0, 55}, 200, 50, 3)) {
            state = check_solution(board, timer.now());
            if (std::holds_alternative<solved_rs>(state)) {
                std::print("solved!\n");
            }
        }

        ui.end_frame(dt);
        renderer.draw(window.width(), window.height());
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