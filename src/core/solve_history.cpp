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
    for (const auto& [pos, change] : event.changes) {
        std::print("    pos = [{}, {}]\n", pos.x, pos.y);
        std::print("        changed = {}\n", change.changed);
        std::print("        from = {}\n", to_string(change.from));
        std::print("        to = {}\n", to_string(change.to));

        std::print("        centre_marks_added:");
        for (const auto val : change.centre_marks_added) {
            std::print(" {}", val);
        }
        std::print("\n");
        std::print("        centre_marks_removed:");
        for (const auto val : change.centre_marks_removed) {
            std::print(" {}", val);
        }
        std::print("\n");

        std::print("        corner_marks_added:");
        for (const auto val : change.corner_marks_added) {
            std::print(" {}", val);
        }
        std::print("\n");
        std::print("        corner_marks_removed:");
        for (const auto val : change.corner_marks_removed) {
            std::print(" {}", val);
        }
        std::print("\n");
    }
}

auto is_empty_event(const edit_event& event) -> bool
{
    for (const auto& [pos, change] : event.changes) {
        if (change.changed) return false;
        if (!change.centre_marks_added.empty()) return false;
        if (!change.centre_marks_removed.empty()) return false;
        if (!change.corner_marks_added.empty()) return false;
        if (!change.corner_marks_removed.empty()) return false;
    }
    return true;
}

}

void solve_history::add_event(const edit_event& event)
{
    if (is_empty_event(event)) return; // don't bother recording empty events
    dump_event(event);
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