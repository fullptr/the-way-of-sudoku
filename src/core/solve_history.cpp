#include "solve_history.hpp"
#include "utility.hpp"

#include <print>

namespace sudoku {

void solve_history::add_event(const edit_event& event)
{
    if (event.empty()) return; // don't bother recording empty events
    if (d_curr < d_events.size()) {
        d_events.resize(d_curr); // if we've gone back and then made an edit, remove all forward history
    }
    d_events.push_back(event);
    if (d_events.size() > 1024) { // only keep the last 1024 history
        d_events.pop_front();
    }
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