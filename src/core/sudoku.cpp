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

auto sudoku_board::for_each_selected(const std::function<void(int, int, sudoku_cell&)>& fn)
{
    for (i32 x = 0; x != d_size; ++x) {
        for (i32 y = 0; y != d_size; ++y) {
            auto& cell = get({x, y});
            if (cell.selected && !cell.fixed) fn(x, y, cell);
        }
    }
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
    auto event = edit_event{};

    for (i32 x = 0; x != d_size; ++x) {
        for (i32 y = 0; y != d_size; ++y) {
            auto& cell = get({x, y});
            if (cell.selected && !cell.fixed) {
                event.changes[{x, y}].changed = true;
                event.changes[{x, y}].from = cell.value;
                event.changes[{x, y}].to = value;
                cell.value = value;
            }
        }
    }

    d_history.add_event(event);
}

auto sudoku_board::set_corner_pencil_mark(i32 value) -> void
{
    auto event = edit_event{};

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

    for_each_selected([&](int x, int y, sudoku_cell& cell) {
        if (add) {
            if (!cell.corner_pencil_marks.contains(value)) {
                event.changes[{x, y}].corner_marks_added.insert({x, y});
                cell.corner_pencil_marks.insert(value);
            }
        } else {
            if (cell.corner_pencil_marks.contains(value)) {
                event.changes[{x, y}].corner_marks_removed.insert({x, y});
                cell.corner_pencil_marks.erase(value);
            }
        }
    });

    d_history.add_event(event);
}

auto sudoku_board::set_centre_pencil_mark(i32 value) -> void
{
    auto event = edit_event{};

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

    for_each_selected([&](int x, int y, sudoku_cell& cell) {
        if (add) {
            if (!cell.centre_pencil_marks.contains(value)) {
                event.changes[{x, y}].centre_marks_added.insert({x, y});
                cell.centre_pencil_marks.insert(value);
            }
        } else {
            if (cell.centre_pencil_marks.contains(value)) {
                event.changes[{x, y}].centre_marks_removed.insert({x, y});
                cell.centre_pencil_marks.erase(value);
            }
        }
    });

    d_history.add_event(event);
}

namespace {

enum class delete_kind
{
    none,
    digit,
    centre,
    corner,
};

}

void sudoku_board::clear_selected()
{
    auto event = edit_event{};
    
    const auto find_deletion_kind = [&] {
        for (i32 x = 0; x != d_size; ++x) {
            for (i32 y = 0; y != d_size; ++y) {
                auto& cell = get({x, y});
                if (cell.value.has_value()) return delete_kind::digit;
                if (!cell.centre_pencil_marks.empty()) return delete_kind::centre;
                if (!cell.corner_pencil_marks.empty()) return delete_kind::corner;
            }
        }
        return delete_kind::none;
    };

    switch (find_deletion_kind()) {
        case delete_kind::digit: {
            for_each_selected([&](int x, int y, sudoku_cell& cell) {
                if (cell.selected && !cell.fixed) {
                    event.changes[{x, y}].changed = true;
                    event.changes[{x, y}].from = cell.value;
                    event.changes[{x, y}].to = {};
                    cell.value = {};
                }
            });
        } break;
        case delete_kind::centre: {
            for_each_selected([&](int x, int y, sudoku_cell& cell) {
                if (cell.selected && !cell.fixed) {
                    for (auto value : cell.centre_pencil_marks) {
                        event.changes[{x, y}].centre_marks_removed.insert(value);
                    }
                    cell.centre_pencil_marks.clear();
                }
            });
        } break;
        case delete_kind::corner: {
            for_each_selected([&](int x, int y, sudoku_cell& cell) {
                if (cell.selected && !cell.fixed) {
                    for (auto value : cell.centre_pencil_marks) {
                        event.changes[{x, y}].corner_marks_removed.insert(value);
                    }
                    cell.corner_pencil_marks.clear();
                }
            });
        } break;
        case delete_kind::none: {} break;
    }

    d_history.add_event(event);
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

auto sudoku_board::valid(glm::ivec2 pos) const -> bool
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