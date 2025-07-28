#pragma once
#include "utility.hpp"
#include "texture.hpp"

#include <glm/glm.hpp>

#include <memory>
#include <unordered_map>
#include <string_view>

namespace sudoku {
    
struct character
{
    glm::ivec2 position;
    glm::ivec2 size;
    glm::ivec2 bearing;
    i32        advance;
};

struct font_atlas
{
    std::unique_ptr<texture_png>        texture;
    std::unordered_map<char, character> chars;
    character                           missing_char;
    i32                                 height; // height of an "a", used for centring

    auto get_character(char c) const -> const character&;
    auto length_of(std::string_view message) -> i32;
};

// TEMP
// This is just a copy of quad_instance from the shape_renderer, should
// we combine these? I'm just making a copy now since I am assuming both will
// iterate in different directions and I don't necessarily want them tied
// together, but I still feel conflicted.
struct ui_graphics_quad
{
    glm::ivec2 top_left;
    int        width;
    int        height;
    float      angle;
    glm::vec4  colour;
    int        use_texture;
    glm::ivec2 uv_pos;
    glm::ivec2 uv_size;

    static void set_buffer_attributes(std::uint32_t vbo);
};

}