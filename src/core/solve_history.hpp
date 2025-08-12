#pragma once
#include "common.hpp"

#include <vector>
#include <optional>
#include <set>
#include <variant>

#include <glm/glm.hpp>

namespace sudoku {

struct digit_diff
{
    std::optional<i32> old_value;
    std::optional<i32> new_value;
};

struct centre_diff
{
    bool          added;
    std::set<i32> values;
};

struct corner_diff
{
    bool          added;
    std::set<i32> values;
};

struct diff
{
    glm::ivec2                                         pos;
    std::variant<digit_diff, corner_diff, centre_diff> data;
};

using edit_event = std::vector<diff>;

class solve_history
{
    std::vector<edit_event> d_events;
    std::size_t             d_curr;

public:
    solve_history() = default;

    void add_event(const edit_event& event);

    auto go_back() -> std::optional<edit_event>;
    auto go_forward() -> std::optional<edit_event>;
};

}