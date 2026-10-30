
add_executable(simulation_perft
    performance/simulation_perft.cpp
)

target_link_libraries(simulation_perft
    PRIVATE
        fluid_simulation
)
