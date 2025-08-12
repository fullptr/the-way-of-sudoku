#include "solve_history.hpp"

#include <print>

namespace sudoku {
namespace {

auto to_string(std::optional<i32> val) -> std::string
{
    if (val) return std::format("{}", *val);
    return "null";
}

void dump_event(const edit_event& event)
{
    std::print("event:\n");
    for (const auto& diff : event.diffs) {
        std::print("  - pos = [{}, {}], kind = {}, old_value = {}, new_value = {}\n", diff.pos.x, diff.pos.y, (i32)diff.kind, to_string(diff.old_value), to_string(diff.new_value));
    }
}

auto is_empty_event(const edit_event& event) -> bool
{
    return event.diffs.empty();
}

}

void solve_history::add_event(const edit_event& event)
{
    if (is_empty_event(event)) return; // don't bother recording empty events
    if (d_curr < d_events.size()) {
        d_events.resize(d_curr); // if we've gone back and then made an edit, remove all forward history
    }
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