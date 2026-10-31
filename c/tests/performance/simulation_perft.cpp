#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>

#include <sim/sim.h>


int main() {
    constexpr std::uint64_t iterations = 1'000'000;


    simulation_init();

    // Warm-up
    for (int i = 0; i < 1000; ++i) {
        simulation_step();
    }

    const auto start = std::chrono::steady_clock::now();

    for (std::uint64_t i = 0; i < iterations; ++i) {
        simulation_step();
    }

    const auto end = std::chrono::steady_clock::now();

    const auto elapsed =
        std::chrono::duration<double>(end - start).count();

    std::printf("Total: %.3f s\n", elapsed);
    std::printf("Average: %.3f ns/step\n",
                elapsed * 1e9 / iterations);

    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
}
