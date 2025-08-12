#pragma once
#include "common.hpp"

#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>

namespace sudoku {

enum class diff_kind
{
    digit,
    centre,
    corner,
};

struct diff
{
    glm::ivec2         pos;
    diff_kind          kind;
    std::optional<i32> old_value;
    std::optional<i32> new_value;
};

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