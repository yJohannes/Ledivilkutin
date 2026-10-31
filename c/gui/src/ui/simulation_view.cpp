#include "simulation_view.h"

#include <cstdint>
#include <algorithm>
#include <cmath>
#include <array>

#include <imgui.h>

#include <sim/sim_config.h>
#include <sim/sim.h>

#include "gui_config.h"

namespace ledv::gui
{
    namespace
    {
        constexpr std::array<float, 4> INTENSITY = {
            0.0f, 0.15f, 0.4f, 0.9f
        };
    }

    void SimulationView::Render()
    {
        ImGui::Begin("Simulation View");

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const ImVec2 origin = ImGui::GetCursorScreenPos();
        const ImVec2 available = ImGui::GetContentRegionAvail();

        // Scale the grid to fit the available content area.
        const float cell_size = std::max(
            0.0f,
            std::min(
                available.x / static_cast<float>(LEDS_X),
                available.y / static_cast<float>(LEDS_Y)
            )
        );

        const ImVec2 grid_size(
            LEDS_X * cell_size,
            LEDS_Y * cell_size
        );

        if (cell_size > 0.0f)
        {
            // Build the LED intensity map.
            std::array<int, LEDS_X * LEDS_Y> intensity_map{};

            for (int i = 0; i < SIM_PARTICLE_COUNT; ++i)
            {
                // Particle coordinates are fixed-point values in [0, 256).
                const int led_x = static_cast<int>(p[i].x) >> 4;
                const int led_y = static_cast<int>(p[i].y) >> 4;

                if (led_x < 0 || led_x >= LEDS_X ||
                    led_y < 0 || led_y >= LEDS_Y)
                {
                    continue;
                }

                const int idx = LEDS_X * led_y + led_x;

                // Particle cell.
                intensity_map[idx] = 3;

                // Right.
                if (led_x + 1 < LEDS_X)
                {
                    int& value = intensity_map[idx + 1];
                    value = std::min(value + 1, 3);
                }

                // Left.
                if (led_x > 0)
                {
                    int& value = intensity_map[idx - 1];
                    value = std::min(value + 1, 3);
                }

                // Down.
                if (led_y + 1 < LEDS_Y)
                {
                    int& value = intensity_map[idx + LEDS_X];
                    value = std::min(value + 1, 3);
                }

                // Up.
                if (led_y > 0)
                {
                    int& value = intensity_map[idx - LEDS_X];
                    value = std::min(value + 1, 3);
                }
            }

            // Draw the LED grid.
            for (int y = 0; y < LEDS_Y; ++y)
            {
                for (int x = 0; x < LEDS_X; ++x)
                {
                    const ImVec2 min(
                        origin.x + x * cell_size,
                        origin.y + y * cell_size
                    );

                    const ImVec2 max(
                        min.x + cell_size,
                        min.y + cell_size
                    );

                    const int idx = y * LEDS_X + x;

                    int intensity_level = intensity_map[idx];

                    ImU32 cell_color = 0;

                    if (intensity_level == 0)
                    {
                        cell_color = gui_cfg.GetBgColor(gui_cfg.bg_color_intensity);
                    }
                    else
                    {
                        cell_color = gui_cfg.GetColor(INTENSITY[intensity_level]);
                    }

                    draw_list->AddRectFilled(
                        min,
                        max,
                        cell_color
                    );

                    draw_list->AddRect(
                        min,
                        max,
                        gui_cfg.GetBgColor(gui_cfg.bg_color_intensity)
                    );
                }
            }

            // Draw particles.
            if (gui_cfg.render_particles)
            {
                const float particle_radius = cell_size / 6.0f;

                for (int i = 0; i < SIM_PARTICLE_COUNT; ++i)
                {
                    const float x = origin.x
                        + (static_cast<float>(p[i].x) / 256.0f)
                            * grid_size.x;

                    const float y = origin.y
                        + (static_cast<float>(p[i].y) / 256.0f)
                            * grid_size.y;

                    if (x < origin.x || x >= origin.x + grid_size.x ||
                        y < origin.y || y >= origin.y + grid_size.y)
                    {
                        continue;
                    }

                    draw_list->AddCircleFilled(
                        ImVec2(x, y),
                        particle_radius,
                        gui_cfg.GetColor(0.25f)
                    );
                }
            }


            // Center of mass and effective radius.
            // Assumes `sim` is the C++ simulation instance.
            const float com_x =
                static_cast<float>(sim_cum_com.x / SIM_PARTICLE_COUNT);

            const float com_y =
                static_cast<float>(sim_cum_com.y / SIM_PARTICLE_COUNT);

            const ImVec2 center(
                origin.x + (com_x / 256.0f) * grid_size.x,
                origin.y + (com_y / 256.0f) * grid_size.y
            );

            draw_list->AddCircleFilled(
                center,
                cell_size / 8.0f,
                IM_COL32(255, 255, 255, 255)
            );

        }

        // Reserve layout space for the grid.
        ImGui::Dummy(grid_size);

        ImGui::End();
    }
}
