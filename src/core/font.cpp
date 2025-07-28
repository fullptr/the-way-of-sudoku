#include "font.hpp"

#include <ranges>
#include <glad/glad.h>

namespace sudoku {

auto font_atlas::get_character(char c) const -> const character&
{
    if (auto it = chars.find(c); it != chars.end()) {
        return it->second;
    }
    return missing_char;
}

auto font_atlas::length_of(std::string_view message) -> i32
{
    if (message.empty()) {
        return 0;
    }
    i32 length = 0;
    for (char c : message | std::views::drop(1)) {
        length += get_character(c).advance;
    }
    length += get_character(message.back()).size.x;
    return length;
}

void ui_graphics_quad::set_buffer_attributes(std::uint32_t vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    for (int i = 1; i != 9; ++i) {
        glEnableVertexAttribArray(i);
        glVertexAttribDivisor(i, 1);
    }
    glVertexAttribIPointer(1, 2, GL_INT, sizeof(ui_graphics_quad), (void*)offsetof(ui_graphics_quad, top_left));
    glVertexAttribIPointer(2, 1, GL_INT, sizeof(ui_graphics_quad), (void*)offsetof(ui_graphics_quad, width));
    glVertexAttribIPointer(3, 1, GL_INT, sizeof(ui_graphics_quad), (void*)offsetof(ui_graphics_quad, height));
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(ui_graphics_quad), (void*)offsetof(ui_graphics_quad, angle));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(ui_graphics_quad), (void*)offsetof(ui_graphics_quad, colour));
    glVertexAttribIPointer(6, 1, GL_INT, sizeof(ui_graphics_quad), (void*)offsetof(ui_graphics_quad, use_texture));
    glVertexAttribIPointer(7, 2, GL_INT, sizeof(ui_graphics_quad), (void*)offsetof(ui_graphics_quad, uv_pos));
    glVertexAttribIPointer(8, 2, GL_INT, sizeof(ui_graphics_quad), (void*)offsetof(ui_graphics_quad, uv_size));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

}