# =========================
# Simulation Library
# =========================

add_library(fluid_simulation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sim_config.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/int_arithmetic.c
)

target_compile_definitions(fluid_simulation
    PUBLIC
        SIM_USE_DYNAMIC
        SIM_INTEGRATION_ENABLED
        SIM_COLLISIONS_ENABLED
        SIM_COM_ENABLED
        SIM_FRICTION_ENABLED
        I2C_PLATFORM_ARDUINO
)

target_include_directories(fluid_simulation
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include

    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/sim
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
