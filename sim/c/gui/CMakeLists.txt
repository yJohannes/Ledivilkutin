include(FetchContent)

# =========================
# OpenGL, GLFW & Dear ImGui library
# =========================

find_package(OpenGL REQUIRED)

## GLFW - windowing library
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(glfw)


## Dear ImGui source
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.9b-docking
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(imgui)


## Dear ImGui library + GLFW and OpenGL3 backends
add_library(imgui
    STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp

        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp

        ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)

target_include_directories(imgui
    PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui
    PUBLIC
        glfw
        OpenGL::GL
)


# =========================
# Simulation GUI
# =========================

add_executable(fluid_simulation_gui
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/src/gl.c
)

target_include_directories(fluid_simulation_gui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include

)

target_link_libraries(fluid_simulation_gui
    PRIVATE
        fluid_simulation
        glfw
        imgui
        opengl32z
)
