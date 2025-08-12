#pragma once
#include <vector>
#include <optional>
#include <unordered_set>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

namespace sudoku {

enum class edit_type
{
    digit,
    corner_pencil_mark,
    centre_pencil_mark,
};

struct edit_event
{
    edit_type                      type;
    bool                           added;
    std::unordered_set<glm::ivec2> positions;  
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