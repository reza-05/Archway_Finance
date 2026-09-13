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
# System Dependencies (GLFW & OpenGL)
# ==========================================================
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

# ==========================================================
# FetchContent: Dear ImGui (v1.90.5)
# ==========================================================
include(FetchContent)

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

# ==========================================================
# Executable Configuration
# ==========================================================

add_executable(archway
    src/main_gui.cpp
    src/core/core_engine.c
    src/core/ledger_engine.c
    src/core/search_filter.c
    database/database.c
)

target_include_directories(archway PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/database
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(archway PRIVATE
    glfw
    OpenGL::GL
    imgui
)

# ==========================================================
# Compiler Flags & OS-Specific Linking
# ==========================================================
if(MSVC)
    target_compile_options(archway PRIVATE /W4)
else()
    target_compile_options(archway PRIVATE -Wall -Wextra)
endif()

# macOS Frameworks
if(APPLE)
    target_compile_definitions(archway PRIVATE GL_SILENCE_DEPRECATION)
    target_link_libraries(archway PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
endif()

# Windows OpenGL linking
if(WIN32)
    target_link_libraries(archway PRIVATE opengl32)
endif()
