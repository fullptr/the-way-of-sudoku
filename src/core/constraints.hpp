#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace sudoku {

class sudoku_board;
class renderer;
class render_config;

class constraint
{
public:
    virtual auto check(const sudoku_board& board) const -> bool = 0;
    virtual auto draw(renderer& r, const render_config& config) const -> void = 0;
    virtual ~constraint() = default;
};

// TODO: Move to separate header
class renban : public constraint
{
    std::vector<glm::ivec2> d_positions;

public:
    renban(const std::vector<glm::ivec2>& positions) : d_positions{positions} {}
    auto check(const sudoku_board& board) const -> bool override;
    auto draw(renderer& r, const render_config& config) const -> void override;
};

}