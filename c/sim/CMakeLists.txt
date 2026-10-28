# =========================
# Simulation Library
# =========================

add_library(fluid_simulation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simulation.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/int_arithmetic.c
)


target_include_directories(fluid_simulation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sim
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
