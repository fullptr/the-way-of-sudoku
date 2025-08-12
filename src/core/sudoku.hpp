#pragma once
#include "common.hpp"
#include "solve_history.hpp"

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
    u64                      d_size;
    std::vector<sudoku_cell> d_cells;
    solve_history            d_history;

    auto get(glm::ivec2 pos) -> sudoku_cell&;

public:
    sudoku_board(u64 size);

    auto at(glm::ivec2 pos) const -> const sudoku_cell&;

    // Selection API
    auto select(glm::ivec2 pos, bool value) -> void;
    auto toggle_selected(glm::ivec2 pos) -> void;
    void unselect_all();

    // Modify the selected cells
    void set_digit(i32 value);
    void set_corner_pencil_mark(i32 value);
    void set_centre_pencil_mark(i32 value);  
    
    // Clears the selected cells. It will clear digits if there are any,
    // followed by centre pencil marks if there are no digits, followed by
    // corner pencil marks if there are no centre pencil marks.
    void clear_selected();

    // History API
    void undo();
    void redo();

    auto size() const -> u64;
    auto valid(glm::ivec2 pos) -> bool;

    auto cells() const -> const std::vector<sudoku_cell>&;
    
    static auto make_board(std::vector<std::string_view> cells, std::vector<std::string_view> regions) -> sudoku_board;
};


}