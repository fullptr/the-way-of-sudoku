#include "ui.hpp"
#include "common.hpp"
#include "utility.hpp"
#include "shape_renderer.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ranges>
#include <print>

namespace sudoku {

ui_engine::ui_engine(shape_renderer* renderer)
    : d_renderer{renderer}
{
}

static auto is_in_region(glm::vec2 pos, glm::vec2 top_left, f32 width, f32 height) -> bool
{
    return top_left.x <= pos.x && pos.x < top_left.x + width
        && top_left.y <= pos.y && pos.y < top_left.y + height;
}

void ui_engine::end_frame(f64 dt)
{
    // Clean out any elements no longer around
    std::erase_if(d_data, [&](auto& elem) {
        return !elem.second.active;
    });

    d_time += dt;
    d_capture_mouse = false;
    
    for (auto& [hash, data] : d_data) {
        data.active = false; // if made next frame, it will activate again
        data.hovered_this_frame = false;
        data.clicked_this_frame = false;
        data.unhovered_this_frame = true;
        data.unclicked_this_frame = true;
        
        if (d_unclicked_this_frame && data.is_clicked()) {
            data.unclicked_this_frame = true;
            data.unclicked_time = d_time;
        }
        
        if (is_in_region(d_mouse_pos, data.top_left, data.width, data.height)) {
            d_capture_mouse = true;

            if (!data.is_hovered()) {
                data.hovered_this_frame = true;
                data.hovered_time = d_time;
            }
            if (d_clicked_this_frame) {
                data.clicked_this_frame = true;
                data.clicked_time = d_time;
            }
        } else if (data.is_hovered()) {
            data.unhovered_this_frame = true;
            data.unhovered_time = d_time;
        }
    }
    
    d_clicked_this_frame = false;
    d_unclicked_this_frame = false;
}

bool ui_engine::on_event(const event& event)
{
    if (const auto e = event.get_if<mouse_moved_event>()) {
        d_mouse_pos = e->pos;
    }
    else if (const auto e = event.get_if<mouse_pressed_event>()) {
        if (e->button == mouse::left && d_capture_mouse) {
            d_clicked_this_frame = true;
            return true;
        }
    }
    else if (const auto e = event.get_if<mouse_released_event>()) {
        d_unclicked_this_frame = true;
    }
    return false;
}

bool ui_engine::button(
    std::string_view msg, glm::ivec2 pos, i32 width, i32 height, i32 scale, const widget_key& key)
{
    const auto& data = get_data(key, pos, width, height);
    
    constexpr auto unhovered_colour = from_hex(0x2c3e50);
    constexpr auto hovered_colour = from_hex(0x34495e);
    constexpr auto clicked_colour = from_hex(0xbdc3c7);

    const auto lerp_time = 0.1;
    
    auto colour = unhovered_colour;
    if (data.is_clicked()) {
        colour = clicked_colour;
    }
    else if (data.is_hovered()) {
        const auto t = std::clamp(data.time_hovered(d_time) / lerp_time, 0.0, 1.0);
        colour = sudoku::lerp(unhovered_colour, hovered_colour, t);
    }
    else if (d_time > lerp_time) { // Don't start the game looking hovered
        const auto t = std::clamp(data.time_unhovered(d_time) / lerp_time, 0.0, 1.0);
        colour = sudoku::lerp(hovered_colour, unhovered_colour, t);
    }
    
    const auto quad = ui_graphics_quad{pos, width, height, 0.0f, colour, 0, {0, 0}, {0, 0}};
    d_renderer->submit_gquad(quad);
    d_renderer->draw_text_box(msg, pos, width, height, scale, from_hex(0xecf0f1));
    return data.clicked_this_frame;
}

void ui_engine::box(glm::ivec2 pos, i32 width, i32 height, const widget_key& key) {
    const auto& data = get_data(key, pos, width, height);
    
    constexpr auto unhovered_colour = from_hex(0x2c3e50);
    constexpr auto hovered_colour = from_hex(0x34495e);
    
    auto colour = unhovered_colour;
    if (data.is_hovered()) {
        colour = hovered_colour;
    }
    
    const auto quad = ui_graphics_quad{pos, width, height, 0.0f, colour, 0, {0, 0}, {0, 0}};
    d_renderer->submit_gquad(quad);
}

void ui_engine::box_centred(glm::ivec2 centre, i32 width, i32 height, const widget_key& key) {
    auto top_left = centre;
    top_left.x -= width / 2;
    top_left.y -= height / 2;
    box(top_left, width, height, key);
}

void ui_engine::text(std::string_view message, glm::ivec2 pos, i32 size, glm::vec4 colour)
{
    d_renderer->draw_text(message, pos, size, colour);
}

void ui_engine::text_box(std::string_view message, glm::ivec2 pos, i32 width, i32 height, i32 scale, glm::vec4 colour)
{
    d_renderer->draw_text_box(message, pos, width, height, scale, colour);
}

void ui_engine::cell(const sudoku_cell& cell, glm::ivec2 coord, glm::ivec2 pos, i32 width, i32 height)
{
    constexpr auto colour_fixed = from_hex(0xecf0f1);
    constexpr auto colour_added = from_hex(0x1abc9c);

    auto cell_centre = pos;
    cell_centre.x += width / 2;
    cell_centre.y += height / 2;
    box_centred(cell_centre, width, height, {static_cast<u64>(10 * coord.x + coord.y)});
    if (cell.value.has_value()) {
        if (cell.fixed) {
            text_box(std::format("{}", *cell.value), pos, width, height, 6, colour_fixed);
        } else {
            text_box(std::format("{}", *cell.value), pos, width, height, 6, colour_added);
        }
    }
}

}