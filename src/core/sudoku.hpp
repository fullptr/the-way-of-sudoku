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
    std::optional<int> value;
    bool               fixed;
};

class sudoku_board
{
    std::unordered_map<glm::ivec2, sudoku_cell> cells;

public:
    auto at(int x, int y) -> sudoku_cell& { return cells[{x, y}]; }
};

auto make_board(std::vector<std::string_view> cells) -> sudoku_board
{
    auto board = sudoku_board{};
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