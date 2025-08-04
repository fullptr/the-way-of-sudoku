#pragma once
#include "renderer.hpp"
#include "sudoku.hpp"
#include "utility.hpp"

#include <variant>
#include <unordered_set>

namespace sudoku {

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

void draw_board(
    renderer& r,
    glm::vec2 screen_dimensions,
    const sudoku_board& board,
    const board_render_state& state,
    const time_point& now
);

}