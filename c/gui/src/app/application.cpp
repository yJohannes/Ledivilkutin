#include "application.h"

#include <cstdint>
#include <cstdlib>
#include <cassert>
#include <chrono>

#include <imgui.h>

#include <iostream>
#include <sim/simulation.h>
#include <sim/config.h>

#include "config.h"
#include "platform/window.h"
#include "ui/simulation_view.h"


namespace ledv::gui
{
    Application::Application(Window& window)
        : window_(window)
    {
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

                    std::cout << ax << ',' << ay << '\n';

                    // scale to int8 and additionally scale to fit 
                    a.x = (int8_t)(ax / 256);
                    a.y = (int8_t)(ay / 256);

                    simulate_frame();
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
