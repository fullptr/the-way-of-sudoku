#include "constraints.hpp"
#include "utility.hpp"
#include "sudoku.hpp"
#include "renderer.hpp"
#include "draw_board.hpp"

#include <set>

namespace sudoku {

auto renban::check(const sudoku_board& board) const -> bool
{
    assert(d_positions.size() > 1);
    std::set<i32> digits;
    for (const auto pos : d_positions) {
        const auto value = board.at(pos).value;
        if (!value.has_value()) return false;
        digits.insert(*value);
    }

    if (digits.size() <= 1) return false;
    auto left = digits.begin();
    auto right = digits.begin();
    ++right;
    while (right != digits.end()) {
        if (*right - *left != 1) return false;
    }

    return true;
}

auto renban::draw(renderer& r, const render_config& config) const -> void
{
    assert(d_positions.size() > 1);
    for (std::size_t i = 0; i != d_positions.size() - 1; ++i) {
        auto a_pos = d_positions[i];
        auto b_pos = d_positions[i + 1];

        const auto a = config.tl + config.cell_size * glm::vec2{a_pos.x + 0.5f, a_pos.y + 0.5f};
        const auto b = config.tl + config.cell_size * glm::vec2{b_pos.x + 0.5f, b_pos.y + 0.5f};
        r.push_line(a, b, from_hex(0xe84393), 4.0f);
    }
}

auto german_whisper::check(const sudoku_board& board) const -> bool
{
    assert(d_positions.size() > 1);
    for (std::size_t i = 0; i != d_positions.size() - 1; ++i) {
        auto a_pos = d_positions[i];
        auto b_pos = d_positions[i + 1];

        const auto& a = board.at(a_pos);
        const auto& b = board.at(b_pos);

        if (!a.value || !b.value) return false;
        if (std::abs(*a.value - *b.value) < 5) return false;
    }
    return true;
}

auto german_whisper::draw(renderer& r, const render_config& config) const -> void
{
    assert(d_positions.size() > 1);
    for (std::size_t i = 0; i != d_positions.size() - 1; ++i) {
        auto a_pos = d_positions[i];
        auto b_pos = d_positions[i + 1];

        const auto a = config.tl + config.cell_size * glm::vec2{a_pos.x + 0.5f, a_pos.y + 0.5f};
        const auto b = config.tl + config.cell_size * glm::vec2{b_pos.x + 0.5f, b_pos.y + 0.5f};
        r.push_line(a, b, from_hex(0x4cd137), 4.0f);
    }
}

}