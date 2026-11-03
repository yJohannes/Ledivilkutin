#include "simulation_view.h"

#include <cstdint>
#include <algorithm>
#include <array>

#include <cstring>
#include <imgui.h>

#include <sim/sim_config.h>
#include <sim/sim.h>

#include "gui_config.h"

namespace ledv::gui
{
    namespace
    {
        constexpr std::array<float, 4> INTENSITY = {
            0.0f, 0.33f, 0.66f, 1.f
            // 0.0f, 0.25f, 0.5f, 1.f
        };

        uint8_t brightness[64];

        uint8_t get_brightness(uint8_t i)
        {
            return (brightness[i >> 2] >> ((i & 3) << 1)) & 3;
        }



        uint8_t masks[4] = {
            0x03, 0x0C, 0x30, 0xC0
        };

        uint8_t increments[4] = {
            0x01, 0x04, 0x10, 0x40
        };

        void add_brightness_v4(uint8_t i)
        {
            uint8_t *p = &brightness[i >> 2];
            uint8_t pos = i & 3;

            uint8_t mask = masks[pos];

            if ((*p & mask) != mask)
                *p += increments[pos];
        }

        void add_brightness(uint8_t i, uint8_t amount)
        {
            uint8_t shift = (i & 3) << 1;
            uint8_t mask = 3 << shift;

            uint8_t value = (brightness[i >> 2] >> shift) & 3;

            if (value < 3)
            {
                value += amount;
                if (value > 3) value = 3;

                brightness[i >> 2] =
                    (brightness[i >> 2] & ~mask) | (value << shift);
            }
        }

        void process_particle(uint8_t x, uint8_t y)
        {
            uint8_t i = y * 16 + x;

            // Set max brightness
            brightness[i >> 2] |= 3 << ((i & 3) << 1);

            int add_amount = 1;

            // if (x > 0)  add_brightness(i - 1, add_amount);
            // if (x < 15) add_brightness(i + 1, add_amount);
            // if (y > 0)  add_brightness(i - 16, add_amount);
            // if (y < 15) add_brightness(i + 16, add_amount);

            if (x > 0)  add_brightness_v4(i - 1);
            if (x < 15) add_brightness_v4(i + 1);
            if (y > 0)  add_brightness_v4(i - 16);
            if (y < 15) add_brightness_v4(i + 16);
        }



        void update_brightness(void)
        {
            // Clear frame buffer
            std::memset(brightness, 0, sizeof(brightness));

            for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
            {
                uint8_t led_x = p[i].x >> 4;
                uint8_t led_y = p[i].y >> 4;

                process_particle(led_x, led_y);
            }
        }


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

            std::memset(brightness, 0, sizeof(brightness));

            for (int i = 0; i < SIM_PARTICLE_COUNT; ++i)
            {
                // Particle coordinates are fixed-point values in [0, 255].
                const uint8_t led_x = p[i].x >> 4;
                const uint8_t led_y = p[i].y >> 4;

                process_particle(led_x, led_y);

                const int idx = LEDS_X * led_y + led_x;

                // Particle cell.
                intensity_map[idx] = 3;

                const int add_val = 1;

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

            std::array<int, 4> brightness_counts{};

            // Count squares at each brightness level.
            for (int i = 0; i < LEDS_X * LEDS_Y; ++i)
            {
                const int brightness_level = get_brightness(i);

                if (brightness_level >= 0 && brightness_level <= 3)
                    brightness_counts[brightness_level]++;
            }

            constexpr float LED_SCALE = 0.35f;

            // Draw the LED grid.
            for (int y = 0; y < LEDS_Y; ++y)
            {
                for (int x = 0; x < LEDS_X; ++x)
                {
                    const ImVec2 cell_min(
                        origin.x + x * cell_size,
                        origin.y + y * cell_size
                    );

                    const float square_size = cell_size * LED_SCALE;
                    const float offset = (cell_size - square_size) * 0.5f;

                    const ImVec2 min(
                        cell_min.x + offset,
                        cell_min.y + offset
                    );

                    const ImVec2 max(
                        min.x + square_size,
                        min.y + square_size
                    );

                    const int idx = y * LEDS_X + x;
                    // int intensity_level = intensity_map[idx];
                    int intensity_level = get_brightness(idx);

                    ImU32 cell_color = intensity_level == 0
                        ? gui_cfg.GetBgColor(gui_cfg.bg_color_intensity)
                        : gui_cfg.GetColor(INTENSITY[intensity_level]);

                    draw_list->AddRectFilled(min, max, cell_color);

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

            {
                const int total_active_squares =
                    LEDS_X * LEDS_Y - brightness_counts[0];

                ImGui::Text("Brightness distribution:");

                ImGui::Text(
                    "Total active squares: %d",
                    total_active_squares
                );

                for (int i = 0; i <= 3; ++i)
                {
                    const float normalized = total_active_squares > 0
                        ? static_cast<float>(brightness_counts[i]) / total_active_squares
                        : 0.0f;

                    ImGui::Text(
                        "Brightness %d: %03d squares | %.4f (%.2f%%)",
                        i,
                        brightness_counts[i],
                        normalized,
                        normalized * 100.0f
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
