#pragma once
#include "buffer.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "common.hpp"
#include "font.hpp"

#include <glm/glm.hpp>

#include <memory>

namespace sudoku {

struct line_instance
{
    glm::vec2 begin;
    glm::vec2 end;
    glm::vec4 begin_colour;
    glm::vec4 end_colour;
    float     thickness;

    static void set_buffer_attributes(u32 vbo);
};

struct circle_instance
{
    glm::vec2 centre;
    float     inner_radius;
    float     outer_radius;
    glm::vec4 begin_colour;
    glm::vec4 end_colour;
    float     angle;

    static void set_buffer_attributes(u32 vbo);
};

struct quad_instance
{
    glm::ivec2 top_left;
    float      width;
    float      height;
    float      angle;
    glm::vec4  colour;

    static void set_buffer_attributes(u32 vbo);
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

class shape_renderer
{
    u32 d_vao;
    u32 d_vbo;
    u32 d_ebo;

    std::vector<quad_instance>    d_quads;
    std::vector<line_instance>    d_lines;
    std::vector<circle_instance>  d_circles;
    std::vector<ui_graphics_quad> d_gquads;

    shader d_quad_shader;
    shader d_line_shader;
    shader d_circle_shader;
    shader d_graphics_quad_shader;

    vertex_buffer d_instances;

    font_atlas d_atlas;

public:
    shape_renderer();
    ~shape_renderer();

    void draw_frame(i32 screen_width, i32 screen_height);

    void draw_rect(glm::vec2 top_left, float width, float height, glm::vec4 colour);
    void draw_quad(glm::vec2 centre, float width, float height, float angle, glm::vec4 colour);
    void draw_line(glm::vec2 begin, glm::vec2 end, glm::vec4 begin_colour, glm::vec4 end_colour, float thickness);
    void draw_line(glm::vec2 begin, glm::vec2 end, glm::vec4 colour, float thickness);
    void draw_circle(glm::vec2 centre, glm::vec4 colour, float radius);
    void draw_annulus(glm::vec2 centre, glm::vec4 colour, float inner_radius, float outer_radius);
    void draw_text(std::string_view message, glm::ivec2 pos, i32 size, glm::vec4 colour);
    void draw_text_box(std::string_view message, glm::ivec2 pos, i32 width, i32 height, i32 scale, glm::vec4 colour);

    auto submit_gquad(const ui_graphics_quad& quad) { d_gquads.push_back(quad); }
};

}