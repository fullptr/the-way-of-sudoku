#include "common.hpp"
#include "input.hpp"
#include "window.hpp"
#include "utility.hpp"
#include "renderer.hpp"
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

auto hovered_cell(sudoku_board& board, const window& w) -> sudoku_cell*
{
    const auto pos = hovered_cell_pos(board, w);
    if (pos.has_value()) {
        return &board.at(pos->x, pos->y);
    }
    return nullptr;
}

// Regular state, still being solved
struct normal_rs
{
};

// Error state for when the board isn't filled
struct empty_cells_rs
{
    time_point time;
    std::unordered_set<glm::ivec2> cells;
};

// Error state for when the board is filled but the constraints dont hold
struct constraint_faiure_rs
{
    // TODO
};

// Success state for a solved grid
struct solved_rs
{
    time_point time;
};

using board_render_state = std::variant<
    normal_rs,
    empty_cells_rs,
    constraint_faiure_rs,
    solved_rs
>;

auto check_solution(const sudoku_board& board, time_point time) -> board_render_state
{
    auto empty_cells = empty_cells_rs{};
    empty_cells.time = time;

    // check for empty cells
    for (i32 row = 0; row != board.size(); ++row) {
        for (i32 col = 0; col != board.size(); ++col) {
            const auto val = board.at(row, col).value;
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
            const auto val = board.at(row, col).value;
            seen.insert(*val);
        }
        if (seen.size() != board.size()) return constraint_faiure_rs{}; // duplicate values in the row
    }

    // check columns
    for (i32 col = 0; col != board.size(); ++col) {
        std::unordered_set<i32> seen; 
        for (i32 row = 0; row != board.size(); ++row) {
            const auto val = board.at(row, col).value;
            seen.insert(*val);
        }
        if (seen.size() != board.size()) return constraint_faiure_rs{}; // duplicate values in the row
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
        if (seen.size() != board.size()) return constraint_faiure_rs{};
    }

    return solved_rs{ .time = time };
}

auto draw_sudoku_board(
    renderer& r,
    const window& w,
    const sudoku_board& board,
    const board_render_state& state,
    const time_point& now
)
    -> void
{
    constexpr auto colour_given_digits = from_hex(0xecf0f1);
    constexpr auto colour_added_digits = from_hex(0x1abc9c);

    constexpr auto colour_cell = from_hex(0x2c3e50);
    constexpr auto colour_cell_hightlighted = from_hex(0x34495e);

    const auto board_size = 0.9f * std::min(w.width(), w.height());
    const auto board_centre = glm::vec2{w.width(), w.height()} / 2.0f;
    const auto cell_size = board_size / board.size();
    const auto top_left = board_centre - glm::vec2{board_size, board_size} / 2.0f;

    r.push_rect(top_left, board_size, board_size, colour_cell);

    for (int y = 0; y != board.size(); ++y) {
        for (int x = 0; x != board.size(); ++x) {
            const auto cell_top_left = top_left + cell_size * glm::vec2{x, y};
            const auto cell_centre = cell_top_left + glm::vec2{cell_size, cell_size} / 2.0f;

            auto cell_colour = colour_cell;
            if (auto inner = std::get_if<empty_cells_rs>(&state)) {
                if (inner->cells.contains(glm::ivec2{x, y})) {
                    const auto t = std::chrono::duration<double>(now - inner->time).count();
                    cell_colour = lerp(from_hex(0xc0392b), cell_colour, t);
                }
            }
            if (cell_colour != colour_cell) {
                r.push_quad(cell_centre, cell_size, cell_size, 0, cell_colour);
            } else if (board.at(x, y).selected) {
                r.push_quad(cell_centre, cell_size, cell_size, 0, colour_cell_hightlighted);
            }

            const auto& cell = board.at(x, y);
            if (cell.value.has_value()) {
                auto colour = cell.fixed ? colour_given_digits : colour_added_digits;
                auto scale = 6;
                if (std::holds_alternative<solved_rs>(state)) {
                    colour = from_hex(0x2ecc71);
                }
                r.push_text_box(std::format("{}", *cell.value), cell_top_left, cell_size, cell_size, scale, colour);
            } else {
                if (!cell.centre_pencil_marks.empty()) {
                    const auto colour = colour_added_digits;
                    auto s = std::string{};
                    for (auto mark : cell.centre_pencil_marks) {
                        s.append(std::to_string(mark));
                    }
                    const auto length = 2 * r.font().length_of(s);
                    const auto scale = length > cell_size ? 1 : 2;
                    r.push_text_box(s, cell_top_left, cell_size, cell_size, scale, colour);
                }
                if (!cell.corner_pencil_marks.empty()) {
                    const auto colour = colour_added_digits;
                    auto s = std::string{};
                    for (auto mark : cell.corner_pencil_marks) {
                        s.append(std::to_string(mark));
                    }
                    const auto length = 2 * r.font().length_of(s);
                    const auto scale = length > cell_size ? 1 : 2;
                    auto pos = cell_top_left;
                    pos.x += (i32)(cell_size * 0.1f);
                    pos.y += (i32)(cell_size * 0.1f) + r.font().height * scale;
                    r.push_text(s, pos, scale, colour);
                }
            }
        }
    }

    // draw boundary
    const auto tl = glm::vec2{top_left};
    const auto tr = glm::vec2{top_left} + glm::vec2{board_size, 0};
    const auto bl = glm::vec2{top_left} + glm::vec2{0, board_size};
    const auto br = glm::vec2{top_left} + glm::vec2{board_size, board_size};

    for (i32 i = 1; i != board.size(); ++i) {
        const auto offset = glm::vec2{0, i * cell_size};
        r.push_line(tl + offset, tr + offset, from_hex(0x34495e), 1.f);
    }
    for (i32 i = 1; i != board.size(); ++i) {
        const auto offset = glm::vec2{i * cell_size, 0};
        r.push_line(tl + offset, bl + offset, from_hex(0x34495e), 1.f);
    }

    r.push_line(tl, tr, from_hex(0xecf0f1), 2.5f);
    r.push_line(tr, br, from_hex(0xecf0f1), 2.5f);
    r.push_line(br, bl, from_hex(0xecf0f1), 2.5f);
    r.push_line(bl, tl, from_hex(0xecf0f1), 2.5f);

    // draw regions
    for (i32 x = 0; x != board.size(); ++x) {
        for (i32 y = 0; y != board.size(); ++y) {
            if (x + 1 < board.size() && board.at(x, y).region != board.at(x + 1, y).region) {
                const auto a = tl + cell_size * glm::vec2{x + 1, y};
                const auto b = tl + cell_size * glm::vec2{x + 1, y + 1};
                r.push_line(a, b, from_hex(0xecf0f1), 1.f);
            }

            if (y + 1 < board.size() && board.at(x, y).region != board.at(x, y + 1).region) {
                const auto a = tl + cell_size * glm::vec2{x,     y + 1};
                const auto b = tl + cell_size * glm::vec2{x + 1, y + 1};
                r.push_line(a, b, from_hex(0xecf0f1), 1.f);
            }
        }
    }
}

void flip(std::set<i32>& ints, i32 value)
{
    if (ints.contains(value)) {
        ints.erase(value);
    } else {
        ints.insert(value);
    }
}

void update_centre_pencil_mark(sudoku_board& board, i32 value)
{
    // If any of the cells can accept the pencil mark, we are adding, otherwise
    // we are removing
    bool add = false;
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            if (!cell.centre_pencil_marks.contains(value)) {
                add = true;
                break;
            }
        }
    }

    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            if (add) {
                cell.centre_pencil_marks.insert(value);
            } else {
                cell.centre_pencil_marks.erase(value);
            }
        }
    }
}

void clear_centre_pencil_mark(sudoku_board& board)
{
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            cell.centre_pencil_marks.clear();
        }
    }
}

void update_corner_pencil_mark(sudoku_board& board, i32 value)
{
    // If any of the cells can accept the pencil mark, we are adding, otherwise
    // we are removing
    bool add = false;
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            if (!cell.corner_pencil_marks.contains(value)) {
                add = true;
                break;
            }
        }
    }

    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            if (add) {
                cell.corner_pencil_marks.insert(value);
            } else {
                cell.corner_pencil_marks.erase(value);
            }
        }
    }
}

void clear_corner_pencil_mark(sudoku_board& board)
{
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            cell.corner_pencil_marks.clear();
        }
    }
}

void set_value(sudoku_board& board, i32 value)
{
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            cell.value = value;
        }
    }
}

void remove_value(sudoku_board& board)
{
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            cell.value = {};
        }
    }
}

auto has_any_value(const sudoku_board& board) -> bool
{
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            if (cell.value.has_value()) return true;
        }
    }
    return false;
}

auto has_any_centre_pencil_marks(const sudoku_board& board) -> bool
{
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            if (!cell.centre_pencil_marks.empty()) return true;
        }
    }
    return false;
}

auto has_any_corner_pencil_marks(const sudoku_board& board) -> bool
{
    for (auto& cell : board.cells()) {
        if (cell.selected && !cell.fixed) {
            if (!cell.corner_pencil_marks.empty()) return true;
        }
    }
    return false;
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

#define LEVEL 2
#if LEVEL == 0
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
#elif LEVEL == 1
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
#elif LEVEL == 2
    auto board = make_board(
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
                auto cell = hovered_cell(board, window);
                if (cell != nullptr) {
                    if (e->button == mouse::left) {
                        if (e->mods & modifier::shift) {
                            cell->selected = !cell->selected;
                        } else {
                            board.clear_selected();
                            cell->selected = true;
                        }
                        mouse_down = cell->selected;
                    }
                } else {
                    board.clear_selected();
                }
            }
            else if (auto e = event.get_if<mouse_released_event>()) {
                if (e->button == mouse::left) {
                    mouse_down = {};
                }
            }
            else if (auto e = event.get_if<mouse_moved_event>()) {
                if (mouse_down.has_value()) {
                    auto cell = hovered_cell(board, window);
                    if (cell != nullptr) {
                        cell->selected = *mouse_down;
                    }
                }
            }
            else if (auto e = event.get_if<keyboard_pressed_event>()) {
                std::optional<i32> value = {};
                switch (e->key) {
                    case keyboard::backspace: {
                        if (has_any_value(board)) {
                            remove_value(board);
                        } else if (has_any_centre_pencil_marks(board)) {
                            clear_centre_pencil_mark(board);
                        } else if (has_any_corner_pencil_marks(board)) {
                            clear_corner_pencil_mark(board);
                        }
                    } break;
                    case keyboard::escape: {
                        board.clear_selected();
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
                    update_centre_pencil_mark(board, *value);
                }
                else if (e->mods & modifier::shift) {
                    update_corner_pencil_mark(board, *value);
                }
                else {
                    set_value(board, *value);
                }
            }
        }

        draw_sudoku_board(renderer, window, board, state, timer.now());
        
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