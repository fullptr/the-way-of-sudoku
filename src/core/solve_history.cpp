#include "solve_history.hpp"

namespace sudoku {

void solve_history::add_event(const edit_event& event)
{
    d_events.resize(d_curr); // if we've gone back and then made an edit, remove all forward history
    d_events.push_back(event);
    d_curr = d_events.size();
}

auto solve_history::go_back() -> std::optional<edit_event>
{
    if (d_curr == 0) { return std::nullopt; }
    --d_curr;
    return d_events.at(d_curr);
}

auto solve_history::go_forward() -> std::optional<edit_event>
{
    if (d_curr == d_events.size()) { return std::nullopt; }
    const auto ret = d_events.at(d_curr);
    ++d_curr;
    return ret;
}

}