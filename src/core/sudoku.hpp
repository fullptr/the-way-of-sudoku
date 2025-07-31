#pragma once
#include <optional>
#include <vector>
#include <unordered_set>
#include <print>
#include <set>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>

namespace sudoku {

struct sudoku_cell
{
    std::optional<i32> value = {};
    bool               fixed = false;
    std::optional<i32> region = {};
    bool               selected = false;

    std::set<i32> corner_pencil_marks;
    std::set<i32> centre_pencil_marks;
};

struct sudoku_region
{
    std::vector<glm::ivec2> cells;
};

class sudoku_board
{
    u64                            d_size;
    std::vector<sudoku_cell>       d_cells;

public:
    sudoku_board(u64 size)
        : d_size{size}, d_cells{size * size}
    {
    }

    auto at(i32 x, i32 y) -> sudoku_cell& {
        assert(valid(x, y));
        return d_cells[x + y * d_size];
    }

    auto at(i32 x, i32 y) const -> const sudoku_cell& {
        assert(valid(x, y));
        return d_cells[x + y * d_size];
    }

    auto size() const -> u64 {
        return d_size;
    }

    auto valid(i32 x, i32 y) -> bool {
        return 0 <= x && x < d_size && 0 <= y && y < d_size;
    }

    auto clear_selected() -> void {
        for (auto& cell : d_cells) cell.selected = false;
    }

    auto cells() -> std::vector<sudoku_cell>& { return d_cells; }
};

inline auto make_board(std::vector<std::string_view> cells, std::vector<std::string_view> regions) -> sudoku_board
{
    const auto size = cells.size();
    if (regions.size() != size) {
        std::print("make_board failed - regions don't align\n");
        std::exit(1);
    }
    for (const auto& row : cells) {
        if (row.size() != size) {
            std::print("make_board failed - not a square!\n");
            std::exit(1);
        }
    }
    auto board = sudoku_board{size};
    for (int y = 0; y != size; ++y) {
        const auto& row = cells[y];
        for (int x = 0; x != row.size(); ++x) {
            if (row[x] != '.') {
                board.at(x, y).value = static_cast<int>(row[x] - '0');
                board.at(x, y).fixed = true;
            }
            board.at(x, y).region = static_cast<i32>(regions[y][x] - '0');
        }
    }
    return board;
}

}