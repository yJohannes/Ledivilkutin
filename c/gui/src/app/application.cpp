#include "application.h"

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <chrono>

#include <format>
#include <imgui.h>

#include <iostream>
#include <sim/sim.h>
#include <sim/sim_config.h>

#include "gui_config.h"
#include "platform/window.h"
#include "ui/simulation_view.h"


namespace ledv::gui
{
    Application::Application(Window& window)
        : window_(window)
    {
        simulation_init();
    }

    int Application::Run()
    {
        using clock = std::chrono::steady_clock;

        auto next_sim_frame = clock::now();

        while (!window_.ShouldClose())
        {
            window_.PollEvents();

            const auto now = clock::now();
            const auto interval = std::chrono::duration<double>(
                1.0 / static_cast<double>(SIM_FPS)
            );

            if (now >= next_sim_frame || gui_cfg.next_frame_requested)
            {
                if (!gui_cfg.simulation_paused || gui_cfg.next_frame_requested)
                {
                    int16_t ax;
                    int16_t ay;
                    serial_controller_.ReadAcceleration(ax, ay);

                    // Scale to int8 and additionally scale to fit
                    // Flip x-coordinates for easier handling of the
                    // physical Arduino board.
                    // Could possibly just bitshift for 2's complement?
                    a.x = (int8_t)(-ax / 256) >> 2; // divide by 4 => [-32, 31]
                    a.y = (int8_t)(ay / 256) >> 2;

                    std::cout << std::format("a_raw:    [{:>6},{:>6}]", ax, ay)
                              << '\n'
                              << std::format("a_scaled: [{:>6},{:>6}]", a.x, a.y)
                              << '\n';

                    // Choose acceleration
                    // This is moved out of simulation_step for performance.
                    if (--sim_accel_timer == 0)
                    {
                        sim_accel_timer = SIM_ACCELERATION_INTERVAL;
                    }
                    else
                    {
                        a.x = 0u;
                        a.y = 0u;
                    }

                    simulation_step();
                    gui_cfg.next_frame_requested = false;
                }

                next_sim_frame = now +
                    std::chrono::duration_cast<clock::duration>(interval);
            }

            window_.BeginFrame();

            ImGui::DockSpaceOverViewport();

            main_view_.Render();
            cfg_view_.Render();

            window_.EndFrame();
        }

        return EXIT_SUCCESS;
    }
}
