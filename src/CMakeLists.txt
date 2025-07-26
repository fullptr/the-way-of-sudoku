cmake_minimum_required(VERSION 3.30)

add_subdirectory(core)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)

add_executable(game game.m.cpp)

target_include_directories(game PUBLIC .)

target_link_libraries(game PRIVATE
    core
    glm::glm
)