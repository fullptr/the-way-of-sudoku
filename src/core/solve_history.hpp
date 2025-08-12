#pragma once
#include "common.hpp"

#include <vector>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace sudoku {

struct value_change
{
    // Digit change
    bool changed = false;
    std::optional<i32> from;
    std::optional<i32> to;

    std::unordered_set<i32> corner_marks_added;
    std::unordered_set<i32> corner_marks_removed;

    std::unordered_set<i32> centre_marks_added;
    std::unordered_set<i32> centre_marks_removed;
};

struct edit_event
{
    std::unordered_map<glm::ivec2, value_change> changes;
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