#include "draw_board.hpp"

namespace sudoku {

auto draw_board(
    renderer& r,
    glm::vec2 screen_dimensions,
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

    const auto board_size = 0.9f * std::min(screen_dimensions.x, screen_dimensions.y);
    const auto board_centre = screen_dimensions / 2.0f;
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

}