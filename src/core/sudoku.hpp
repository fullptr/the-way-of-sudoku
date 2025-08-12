#pragma once
#include "common.hpp"

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
    sudoku_board(u64 size);

    auto at(i32 x, i32 y) -> sudoku_cell&;
    auto at(i32 x, i32 y) const -> const sudoku_cell&;

    auto size() const -> u64;

    auto valid(i32 x, i32 y) -> bool;

    auto clear_selected() -> void;

    auto cells() -> std::vector<sudoku_cell>&;
    auto cells() const -> const std::vector<sudoku_cell>&;
};

auto make_board(std::vector<std::string_view> cells, std::vector<std::string_view> regions) -> sudoku_board;

}