#include "sudoku.hpp"
#include "renderer.hpp"

namespace sudoku {

sudoku_board::sudoku_board(u64 size)
    : d_size{size}, d_cells{size * size}
{
}

auto sudoku_board::at(i32 x, i32 y) -> sudoku_cell& {
    assert(valid(x, y));
    return d_cells[x + y * d_size];
}

auto sudoku_board::at(i32 x, i32 y) const -> const sudoku_cell& {
    assert(valid(x, y));
    return d_cells[x + y * d_size];
}

auto sudoku_board::size() const -> u64 {
    return d_size;
}

auto sudoku_board::valid(i32 x, i32 y) -> bool {
    return 0 <= x && x < d_size && 0 <= y && y < d_size;
}

auto sudoku_board::clear_selected() -> void {
    for (auto& cell : d_cells) cell.selected = false;
}

auto sudoku_board::cells() -> std::vector<sudoku_cell>& { return d_cells; }
auto sudoku_board::cells() const -> const std::vector<sudoku_cell>& { return d_cells; }

auto make_board(std::vector<std::string_view> cells, std::vector<std::string_view> regions) -> sudoku_board
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