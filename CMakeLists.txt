cmake_minimum_required(VERSION 3.16)
project(subdivision_visualizer
    VERSION 0.1
    DESCRIPTION "Interactive 2D subdivision visualizer (Chaikin & four-point)"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

# --- Vendored Dear ImGui (v1.92.8) with SDL2 + SDLRenderer2 backends -------
add_library(imgui STATIC
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_sdl2.cpp
    external/imgui/backends/imgui_impl_sdlrenderer2.cpp)
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends)
target_link_libraries(imgui PUBLIC SDL2::SDL2)

# --- Application ------------------------------------------------------------
add_executable(subdivision_visualizer
    src/main.cpp
    src/canvas.cpp
    src/viewport.cpp)
target_include_directories(subdivision_visualizer PRIVATE include)
target_compile_options(subdivision_visualizer PRIVATE -Wall -Wextra)
target_link_libraries(subdivision_visualizer PRIVATE imgui SDL2::SDL2)

# --- Tests (math core only — no SDL/ImGui dependency) -----------------------
enable_testing()
add_executable(test_subdivision tests/test_subdivision.cpp)
target_include_directories(test_subdivision PRIVATE include)
target_compile_options(test_subdivision PRIVATE -Wall -Wextra)
add_test(NAME subdivision_math COMMAND test_subdivision)
