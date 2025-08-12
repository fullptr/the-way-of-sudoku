#include "sudoku.hpp"
#include "renderer.hpp"

namespace sudoku {

sudoku_board::sudoku_board(u64 size)
    : d_size{size}, d_cells{size * size}
{
}

auto sudoku_board::get(glm::ivec2 pos) -> sudoku_cell&
{
    assert(valid(pos));
    return d_cells[pos.x + pos.y * d_size];
}

auto sudoku_board::at(glm::ivec2 pos) const -> const sudoku_cell&
{
    assert(valid(pos));
    return d_cells[pos.x + pos.y * d_size];
}

auto sudoku_board::select(glm::ivec2 pos, bool value) -> void
{
    assert(valid(pos));
    get(pos).selected = value;
}

auto sudoku_board::toggle_selected(glm::ivec2 pos) -> void
{
    assert(valid(pos));
    auto& cell = get(pos);
    cell.selected = !cell.selected;
}

auto sudoku_board::set_digit(i32 value) -> void
{
    for (auto& cell : d_cells) {
        if (cell.selected && !cell.fixed) {
            cell.value = value;
        }
    }
}

auto sudoku_board::set_corner_pencil_mark(i32 value) -> void
{
    // If any of the cells can accept the pencil mark, we are adding, otherwise
    // we are removing
    bool add = false;
    for (auto& cell : d_cells) {
        if (cell.selected && !cell.fixed) {
            if (!cell.corner_pencil_marks.contains(value)) {
                add = true;
                break;
            }
        }
    }

    for (auto& cell : d_cells) {
        if (cell.selected && !cell.fixed) {
            if (add) {
                cell.corner_pencil_marks.insert(value);
            } else {
                cell.corner_pencil_marks.erase(value);
            }
        }
    }
}

auto sudoku_board::set_centre_pencil_mark(i32 value) -> void
{
    // If any of the cells can accept the pencil mark, we are adding, otherwise
    // we are removing
    bool add = false;
    for (auto& cell : d_cells) {
        if (cell.selected && !cell.fixed) {
            if (!cell.centre_pencil_marks.contains(value)) {
                add = true;
                break;
            }
        }
    }

    for (auto& cell : d_cells) {
        if (cell.selected && !cell.fixed) {
            if (add) {
                cell.centre_pencil_marks.insert(value);
            } else {
                cell.centre_pencil_marks.erase(value);
            }
        }
    }
}

namespace { // TODO: Merge all this logic together

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

void sudoku_board::clear_selected()
{
    if (has_any_value(*this)) {
        for (auto& cell : d_cells) {
            if (cell.selected && !cell.fixed) {
                cell.value = {};
            }
        }
    } else if (has_any_centre_pencil_marks(*this)) {
        for (auto& cell : d_cells) {
            if (cell.selected && !cell.fixed) {
                cell.centre_pencil_marks.clear();
            }
        }
    } else if (has_any_corner_pencil_marks(*this)) {
        for (auto& cell : d_cells) {
            if (cell.selected && !cell.fixed) {
                cell.corner_pencil_marks.clear();
            }
        }
    }
}

void sudoku_board::undo()
{

}

void sudoku_board::redo()
{

}

auto sudoku_board::size() const -> u64
{
    return d_size;
}

auto sudoku_board::valid(glm::ivec2 pos) -> bool
{
    return 0 <= pos.x && pos.x < d_size && 0 <= pos.y && pos.y < d_size;
}

auto sudoku_board::unselect_all() -> void 
{
    for (auto& cell : d_cells) cell.selected = false;
}

auto sudoku_board::cells() const -> const std::vector<sudoku_cell>&
{
    return d_cells;
}

auto sudoku_board::make_board(std::vector<std::string_view> cells, std::vector<std::string_view> regions) -> sudoku_board
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
                board.get({x, y}).value = static_cast<int>(row[x] - '0');
                board.get({x, y}).fixed = true;
            }
            board.get({x, y}).region = static_cast<i32>(regions[y][x] - '0');
        }
    }
    return board;
}

}