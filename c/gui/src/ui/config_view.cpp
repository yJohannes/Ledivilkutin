#include "config_view.h"

#include <cstdint>

#include <imgui.h>

#include <sim/sim_config.h>

#include "gui_config.h"


namespace ledv::gui
{
    void ConfigView::Render()
    {
        ImGui::Begin("Simulation Configuration");

        // FPS
        ImGui::SeparatorText("Simulation");

        {
            if (ImGui::Button(gui_cfg.simulation_paused ? "Resume Simulation" : "Pause Simulation"))
            {
                gui_cfg.simulation_paused = !gui_cfg.simulation_paused;
            }
        }

        {
            if (ImGui::Button("Next Frame"))
            {
                gui_cfg.next_frame_requested = true;
            }
        }

        {
            uint8_t min = 1;
            uint8_t max = 60;

            ImGui::SliderScalar(
                "FPS",
                ImGuiDataType_U8,
                &cfg.fps,
                &min,
                &max
            );
        }

        // Particles
        ImGui::SeparatorText("Particles");

        {
            ImGui::Checkbox("Render Particles", &gui_cfg.render_particles);
        }

        {
            uint8_t min = 1;
            uint8_t max = 127;

            ImGui::SliderScalar(
                "Radius",
                ImGuiDataType_U8,
                &cfg.particle_radius,
                &min,
                &max
            );
        }

        {
            uint8_t min = 3;
            uint8_t max = 127;

            ImGui::SliderScalar(
                "Minimum Distance",
                ImGuiDataType_U8,
                &cfg.particle_min_distance,
                &min,
                &max
            );
        }

        {
            uint8_t min = 0;
            uint8_t max = 255;

            ImGui::SliderScalar(
                "Maximum Displacement",
                ImGuiDataType_U8,
                &cfg.particle_max_displacement,
                &min,
                &max
            );
        }

        // Acceleration
        ImGui::SeparatorText("Acceleration");

        {
            uint8_t min = 1;
            uint8_t max = 10;

            ImGui::SliderScalar(
                "Interval##Acceleration",
                ImGuiDataType_U8,
                &cfg.acceleration_interval,
                &min,
                &max
            );
        }

        {
            uint8_t min = 0;
            uint8_t max = cfg.acceleration_interval - 1;

            ImGui::SliderScalar(
                "Remainder##Acceleration",
                ImGuiDataType_U8,
                &cfg.acceleration_remainder,
                &min,
                &max
            );
        }

        // Center of mass
        ImGui::SeparatorText("Center of Mass");

        {
            uint8_t min = 1;
            uint8_t max = 10;

            ImGui::SliderScalar(
                "Interval##CenterOfMass",
                ImGuiDataType_U8,
                &cfg.center_of_mass_interval,
                &min,
                &max
            );
        }

        {
            uint8_t min = 0;
            uint8_t max = cfg.center_of_mass_interval - 1;

            ImGui::SliderScalar(
                "Remainder##CenterOfMass",
                ImGuiDataType_U8,
                &cfg.center_of_mass_remainder,
                &min,
                &max
            );
        }

        // Friction
        ImGui::SeparatorText("Friction");

        {
            uint8_t min = 1;
            uint8_t max = 10;

            ImGui::SliderScalar(
                "Interval##Friction",
                ImGuiDataType_U8,
                &cfg.friction_interval,
                &min,
                &max
            );
        }

        {
            uint8_t min = 0;
            uint8_t max = cfg.friction_interval - 1;

            ImGui::SliderScalar(
                "Remainder##Friction",
                ImGuiDataType_U8,
                &cfg.friction_remainder,
                &min,
                &max
            );
        }

        {
            ImGui::ColorPicker3("Color", gui_cfg.base_color_float3);
            ImGui::SliderFloat("Color Intensity", &gui_cfg.color_intensity, 0.0f, 1.0f);
            gui_cfg.color = gui_cfg.GetColor(gui_cfg.color_intensity);
        }

        {
            ImGui::ColorPicker3("Background Color", gui_cfg.bg_base_color_float3);
            ImGui::SliderFloat("Background Intensity", &gui_cfg.bg_color_intensity, 0.0f, 1.0f);
            gui_cfg.bg_color = gui_cfg.GetColor(gui_cfg.bg_color_intensity);
        }


        ImGui::End();
    }
}



