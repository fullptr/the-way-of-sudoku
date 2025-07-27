#pragma once
#include <optional>
#include <vector>
#include <print>

#include <glm/glm.hpp>

namespace sudoku {

struct sudoku_cell
{
    std::optional<int> value = {};
    bool               fixed = false;
};

class sudoku_board
{
    u64                      d_size;
    std::vector<sudoku_cell> d_cells;

public:
    sudoku_board(u64 size)
        : d_size{size}, d_cells{size * size}
    {
    }

    auto at(u64 x, u64 y) -> sudoku_cell& {
        assert(x < d_size);
        assert(y < d_size);
        return d_cells[x + y * d_size];
    }

    auto size() const -> u64 {
        return d_size;
    }
};

inline auto make_board(std::vector<std::string_view> cells) -> sudoku_board
{
    const auto size = cells.size();
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
        }
    }
    return board;
}

}