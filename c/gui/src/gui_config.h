#pragma once

#include <algorithm>

#include <imgui.h>


struct GuiConfig
{
    float base_color_float3[3] =
    {
        255.0f / 255.0f,
        141.0f / 255.0f,
        54.0f  / 255.0f
    };

    float bg_base_color_float3[3] =
    {
        70.f / 255.f,
        68.f / 255.f,
        64.f / 255.f
    };

    float color_intensity = 0.85;
    float bg_color_intensity = 0.05;

    ImU32 color = 0u;
    ImU32 bg_color = 0u;


    bool simulation_paused = false;
    bool next_frame_requested = false;
    bool render_particles = true;

    ImU32 GetColor(float intensity = 1.0f) const
    {
        intensity = std::clamp(intensity, 0.0f, 1.0f);

        return IM_COL32(
            static_cast<int>(base_color_float3[0] * intensity * 255.0f),
            static_cast<int>(base_color_float3[1] * intensity * 255.0f),
            static_cast<int>(base_color_float3[2] * intensity * 255.0f),
            255
        );
    }

    ImU32 GetBgColor(float intensity = 1.0f) const
    {
        intensity = std::clamp(intensity, 0.0f, 1.0f);

        return IM_COL32(
            static_cast<int>(bg_base_color_float3[0] * intensity * 255.0f),
            static_cast<int>(bg_base_color_float3[1] * intensity * 255.0f),
            static_cast<int>(bg_base_color_float3[2] * intensity * 255.0f),
            255
        );
    }
};


inline GuiConfig gui_cfg;
