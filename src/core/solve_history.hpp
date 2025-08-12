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
    glm::ivec2 pos;
    std::optional<i32> old_value;
    std::optional<i32> new_value;
};

struct centre_diff
{
    glm::ivec2    pos;
    bool          added;
    std::set<i32> values;
};

struct corner_diff
{
    glm::ivec2    pos;
    bool          added;
    std::set<i32> values;
};

using diff = std::variant<digit_diff, corner_diff, centre_diff>;

struct edit_event
{
    std::vector<diff> diffs;
};

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