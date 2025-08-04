#include "draw_board.hpp"

#include <ranges>

namespace sudoku {
namespace {

constexpr auto colour_given_digits = from_hex(0xecf0f1);
constexpr auto colour_added_digits = from_hex(0x1abc9c);

constexpr auto colour_cell = from_hex(0x2c3e50);
constexpr auto colour_cell_hightlighted = from_hex(0x34495e);

struct render_config
{
    f32       board_size;
    glm::vec2 board_centre;

    f32 cell_size;

    glm::vec2 tl;
    glm::vec2 tr;
    glm::vec2 bl;
    glm::vec2 br;
};

// draw backboard
auto draw_backboard(renderer& r, glm::vec2 screen_dimensions, const sudoku_board& board, const render_config& config)
{
    r.push_rect(config.tl, config.board_size, config.board_size, colour_cell);

    for (i32 i = 1; i != board.size(); ++i) {
        const auto offset = glm::vec2{0, i * config.cell_size};
        r.push_line(config.tl + offset, config.tr + offset, from_hex(0x34495e), 1.f);
    }
    for (i32 i = 1; i != board.size(); ++i) {
        const auto offset = glm::vec2{i * config.cell_size, 0};
        r.push_line(config.tl + offset, config.bl + offset, from_hex(0x34495e), 1.f);
    }
}

// draw highlighted
auto draw_highlighted(renderer& r, glm::vec2 screen_dimensions, const sudoku_board& board, const render_config& config)
{
    for (int y = 0; y != board.size(); ++y) {
        for (int x = 0; x != board.size(); ++x) {
            if (board.at(x, y).selected) {
                const auto cell_centre = config.tl + config.cell_size * glm::vec2{x + 0.5f, y + 0.5f};
                r.push_quad(cell_centre, config.cell_size, config.cell_size, 0, colour_cell_hightlighted);
            }
        }
    }
}

// draw constraints
auto draw_constraints(renderer& r, glm::vec2 screen_dimensions, const sudoku_board& board, const board_render_state& state, const render_config& config, const time_point& now)
{
    // should this be elsewhere? probably...
    if (auto inner = std::get_if<empty_cells_rs>(&state)) {
        for (int y = 0; y != board.size(); ++y) {
            for (int x = 0; x != board.size(); ++x) {
                
                if (inner->cells.contains(glm::ivec2{x, y})) {
                    const auto t = std::chrono::duration<double>(now - inner->time).count();
                    auto cell_colour = lerp(from_hex(0xc0392b), colour_cell, t);
                    const auto cell_centre = config.tl + config.cell_size * glm::vec2{x + 0.5f, y + 0.5f};
                    r.push_quad(cell_centre, config.cell_size, config.cell_size, 0, cell_colour);
                }
            }
        }
    }

    // draw the boundaries of the regions
    for (i32 x = 0; x != board.size(); ++x) {
        for (i32 y = 0; y != board.size(); ++y) {
            if (x + 1 < board.size() && board.at(x, y).region != board.at(x + 1, y).region) {
                const auto a = config.tl + config.cell_size * glm::vec2{x + 1, y};
                const auto b = config.tl + config.cell_size * glm::vec2{x + 1, y + 1};
                r.push_line(a, b, from_hex(0xecf0f1), 1.f);
            }

            if (y + 1 < board.size() && board.at(x, y).region != board.at(x, y + 1).region) {
                const auto a = config.tl + config.cell_size * glm::vec2{x,     y + 1};
                const auto b = config.tl + config.cell_size * glm::vec2{x + 1, y + 1};
                r.push_line(a, b, from_hex(0xecf0f1), 1.f);
            }
        }
    }
}

// draw border
auto draw_border(renderer& r, const render_config& config)
{
    r.push_line(config.tl, config.tr, from_hex(0xecf0f1), 2.5f);
    r.push_line(config.tr, config.br, from_hex(0xecf0f1), 2.5f);
    r.push_line(config.br, config.bl, from_hex(0xecf0f1), 2.5f);
    r.push_line(config.bl, config.tl, from_hex(0xecf0f1), 2.5f);
}

// draw digits
auto draw_digits(renderer& r, const sudoku_board& board, const board_render_state& state, const render_config& config)
{
    for (int y = 0; y != board.size(); ++y) {
        for (int x = 0; x != board.size(); ++x) {
            const auto cell_top_left = config.tl + config.cell_size * glm::vec2{x, y};
            const auto cell_centre = cell_top_left + glm::vec2{config.cell_size, config.cell_size} / 2.0f;

            const auto& cell = board.at(x, y);
            if (cell.value.has_value()) {
                auto colour = cell.fixed ? colour_given_digits : colour_added_digits;
                auto scale = 6;
                if (std::holds_alternative<solved_rs>(state)) {
                    colour = from_hex(0x2ecc71);
                }
                r.push_text_box(std::format("{}", *cell.value), cell_top_left, config.cell_size, config.cell_size, scale, colour);
                continue; // only render the main digit if it's given
            }

            const auto colour = colour_added_digits;

            if (!cell.centre_pencil_marks.empty()) {
                auto s = std::string{};
                for (auto mark : cell.centre_pencil_marks) {
                    s.append(std::to_string(mark));
                }
                const auto length = 2 * r.font().length_of(s);
                const auto scale = length > config.cell_size ? 1 : 2;
                r.push_text_box(s, cell_top_left, config.cell_size, config.cell_size, scale, colour);
            }

            if (!cell.corner_pencil_marks.empty()) {
                auto s = std::string{};
                for (auto mark : cell.corner_pencil_marks) {
                    s.append(std::to_string(mark));
                }
                const auto length = 2 * r.font().length_of(s);
                const auto scale = length > config.cell_size ? 1 : 2;
                auto pos = cell_top_left;
                pos.x += (i32)(config.cell_size * 0.1f);
                pos.y += (i32)(config.cell_size * 0.1f) + r.font().height * scale;
                r.push_text(s, pos, scale, colour);
            }
        }
    }
}

}

void draw_board(
    renderer& r,
    glm::vec2 screen_dimensions,
    const sudoku_board& board,
    const board_render_state& state,
    const time_point& now)
{
    const auto board_size = 0.9f * std::min(screen_dimensions.x, screen_dimensions.y);
    const auto board_centre = screen_dimensions / 2.0f;
    const auto top_left = board_centre - glm::vec2{board_size, board_size} / 2.0f;

    const auto config = render_config{
        .board_size = board_size,
        .board_centre = board_centre,
        .cell_size = board_size / board.size(),
        .tl = top_left,
        .tr = top_left + glm::vec2{board_size, 0},
        .bl = top_left + glm::vec2{0, board_size},
        .br = top_left + glm::vec2{board_size, board_size}
    };

    draw_backboard(r, screen_dimensions, board, config);
    draw_highlighted(r, screen_dimensions, board, config);
    draw_constraints(r, screen_dimensions, board, state, config, now);
    draw_border(r, config);
    draw_digits(r, board, state, config);
}

}