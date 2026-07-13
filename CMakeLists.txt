cmake_minimum_required(VERSION 3.20)

project(
    Archway
    VERSION 1.0
    LANGUAGES C CXX
)

# ==========================================================
# C/C++ Standard
# ==========================================================

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ==========================================================
# FetchContent
# ==========================================================

include(FetchContent)

# ==========================================================
# GLM
# ==========================================================

find_package(glm CONFIG REQUIRED)

# ==========================================================
# GLFW (installed via MSYS2/Linux package manager)
# ==========================================================

find_package(glfw3 REQUIRED)

# ==========================================================
# OpenGL
# ==========================================================

find_package(OpenGL REQUIRED)

# ==========================================================
# Dear ImGui
# ==========================================================

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

target_link_libraries(imgui PUBLIC
    glfw
    OpenGL::GL
)

# ==========================================================
# ImPlot
# ==========================================================

FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v0.16
)

FetchContent_MakeAvailable(implot)

add_library(implot STATIC
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
)

target_include_directories(implot PUBLIC
    ${implot_SOURCE_DIR}
)

target_link_libraries(implot PUBLIC
    imgui
)

# ==========================================================
# Project Sources
# ==========================================================

file(GLOB_RECURSE BACKEND_SOURCES
    CONFIGURE_DEPENDS
    src/backend/*.c
)

file(GLOB_RECURSE FRONTEND_SOURCES
    CONFIGURE_DEPENDS
    src/gui/*.cpp
)

# ==========================================================
# Executable
# ==========================================================

add_executable(archway
    src/main.cpp

    ${BACKEND_SOURCES}
    ${FRONTEND_SOURCES}

    third_party/sqlite/sqlite3.c
)

# ==========================================================
# Include Directories
# ==========================================================

target_include_directories(archway PRIVATE
    src/backend
    src/gui

    third_party/sqlite
)

# ==========================================================
# Libraries
# ==========================================================

target_link_libraries(archway PRIVATE
    glfw
    OpenGL::GL

    glm::glm
    imgui
    implot
)

# ==========================================================
# Compiler Warnings
# ==========================================================

if(MSVC)
    target_compile_options(archway PRIVATE
        /W4
    )
else()
    target_compile_options(archway PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# ==========================================================
# Windows
# ==========================================================

if(WIN32)
    target_link_libraries(archway PRIVATE
        opengl32
    )
endif()

# ==========================================================
# Linux
# ==========================================================

if(UNIX AND NOT APPLE)
    target_link_libraries(archway PRIVATE
        pthread
        dl
    )
endif()

# ==========================================================
# macOS
# ==========================================================

if(APPLE)
    target_link_libraries(archway PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()