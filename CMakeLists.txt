cmake_minimum_required(VERSION 3.20)

project(
    Archway
    VERSION 1.0
    LANGUAGES C CXX
)

# Export compile commands for VS Code IntelliSense (clangd / C/C++ extension)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ==========================================================
# C/C++ Standard
# ==========================================================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================================
# System Dependencies (Installed via MSYS2 pacman)
# ==========================================================
find_package(glm CONFIG REQUIRED)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# ==========================================================
# FetchContent: Dear ImGui
# ==========================================================
include(FetchContent)

# 1. Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.5
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC 
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

# 2. ImPlot
FetchContent_Declare(
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v0.16
)
FetchContent_MakeAvailable(implot)

add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
)
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

# ==========================================================
# Executable Configuration
# ==========================================================

# Explicitly defining source files prevents caching issues
add_executable(archway
    src/main.cpp
    src/ui_dashboard.cpp
)

target_include_directories(archway PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(archway PRIVATE
    glfw
    OpenGL::GL
    glm::glm
    imgui
)

# ==========================================================
# Compiler Flags & OS Specific Linking
# ==========================================================
if(MSVC)
    target_compile_options(archway PRIVATE /W4)
else()
    target_compile_options(archway PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Windows/MSYS2 specific OpenGL linking
if(WIN32)
    target_link_libraries(archway PRIVATE opengl32)
endif()