#pragma once
#include <optional>
#include <unordered_map>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/hash.hpp>

namespace sudoku {

struct sudoku_cell
{
    std::optional<int> value = {};
    bool               fixed = false;
};

class sudoku_board
{
    u64 d_size;
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
};

auto make_board(std::vector<std::string_view> cells) -> sudoku_board
{
    auto board = sudoku_board{9};
    for (int y = 0; y != cells.size(); ++y) {
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