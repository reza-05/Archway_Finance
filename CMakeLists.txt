cmake_minimum_required(VERSION 3.20)
project(archway LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Enable FetchContent
include(FetchContent)

# Use macOS OpenGL 3 Core Profile headers and silence legacy warnings
add_compile_definitions(GLFW_INCLUDE_GLCOREARB GL_SILENCE_DEPRECATION)

# 1. Fetch GLM (Header-only math)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)
FetchContent_MakeAvailable(glm)

# 2. Find GLFW (Installed via Homebrew)
find_package(glfw3 REQUIRED)

# 3. Fetch Dear ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.5
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

# We need to compile ImGui files manually
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/src/backend  # macOS Sequoia-তে missing CarbonCore/Script.h হেডার এরর এড়াতে যুক্ত করা হয়েছে
)
target_link_libraries(imgui PUBLIC glfw)

# 4. Fetch ImPlot
FetchContent_Declare(
    implot
    GIT_REPOSITORY https://github.com/epezent/implot.git
    GIT_TAG v0.16
)
FetchContent_GetProperties(implot)
if(NOT implot_POPULATED)
    FetchContent_Populate(implot)
endif()

set(IMPLOT_SOURCES
    ${implot_SOURCE_DIR}/implot.cpp
    ${implot_SOURCE_DIR}/implot_items.cpp
    ${implot_SOURCE_DIR}/implot_demo.cpp
)
add_library(implot STATIC ${IMPLOT_SOURCES})
target_include_directories(implot PUBLIC ${implot_SOURCE_DIR})
target_link_libraries(implot PUBLIC imgui)

# 5. SQLite3 (System package)
find_package(SQLite3 REQUIRED)

# 6. Define Archway Application
# Include backend C files and frontend C++ files
file(GLOB_RECURSE BACKEND_SOURCES src/backend/*.c src/backend/*.h)
file(GLOB_RECURSE FRONTEND_SOURCES src/gui/*.cpp src/gui/*.hpp)

add_executable(archway
    src/main.cpp
    ${BACKEND_SOURCES}
    ${FRONTEND_SOURCES}
)

# Include paths
target_include_directories(archway PRIVATE
    src/backend
    src/gui
)

# Link Libraries
target_link_libraries(archway PRIVATE
    imgui
    implot
    glm
    SQLite3::SQLite3
)

# Platform-specific linking on macOS
if(APPLE)
    target_link_libraries(archway PRIVATE
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework OpenGL"
    )
endif()