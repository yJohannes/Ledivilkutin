#include "simulation.h"

#include <stdint.h>
#include <stdbool.h>

#include "config.h"
#include "saturation_arithmetic.h"
#include "vec2.h"

#if SIM_USE_EUCLIDEAN_SEPARATION
#include "utils.h"
#endif


vec2u p[SIM_PARTICLE_COUNT];
vec2u pp[SIM_PARTICLE_COUNT];
vec2i a;
vec2u16 cum_com;
uint8_t frame_counter;


void initialize_positions(void)
{
    for (uint8_t i = 0; i < SIM_PARTICLE_COUNT; i++)
    {
        int8_t x = (i % 9) - 4;
        int8_t y = (i / 9) - 4;

        p[i] = (vec2u){
            SIM_CENTER_X + x,
            SIM_CENTER_Y + y
        };

        pp[i] = p[i];
    }
}


void simulate_frame(void)
{
    ++frame_counter;

    // Choose acceleration
    int8_t ax = 0u;
    int8_t ay = 0u;

    if (frame_counter % SIM_ACCELERATION_INTERVAL == SIM_ACCELERATION_REMAINDER)
    {
        ax = a.x;
        ay = a.y;
    }

    // Integrate positions using Verlet integration
    for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
    {
        // int8_t diff = (int8_t)(a - b);
        // For 2's complement the underflow + cast gives the
        // correct result :)
        int8_t dx = p[i].x - pp[i].x;
        int8_t dy = p[i].y - pp[i].y;

        // Clamp dx
        if (dx > SIM_PARTICLE_MAX_DISPLACEMENT)
            dx = SIM_PARTICLE_MAX_DISPLACEMENT;
        else if (dx < -SIM_PARTICLE_MAX_DISPLACEMENT)
            dx = -SIM_PARTICLE_MAX_DISPLACEMENT;

        if (dy > SIM_PARTICLE_MAX_DISPLACEMENT)
            dy = SIM_PARTICLE_MAX_DISPLACEMENT;
        else if (dy < -SIM_PARTICLE_MAX_DISPLACEMENT)
            dy = -SIM_PARTICLE_MAX_DISPLACEMENT;

        // Save previous position before overwriting current.
        pp[i].x = p[i].x;
        pp[i].y = p[i].y;

        // new_x = x + dx + a
        // trust that dx + a.x won't over/underflow :)
        // clamp nx to [0, 255]
        p[i].x = sat_add_u8_s8(p[i].x, dx + ax);
        p[i].y = sat_add_u8_s8(p[i].y, dy + ay);

        // uint8_t nx = sat_add_u8_s8(p[i].x, dx + a.x);
        // uint8_t ny = sat_add_u8_s8(p[i].y, dy + a.y);
    }

    // Handle overlaps
    for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
    {
        // Handle each pair only
        for (uint8_t j = i + 1; j < SIM_PARTICLE_COUNT; ++j)
        {
            // TODO: optimize here possibly
            int16_t dx = p[j].x - p[i].x;
            int16_t dy = p[j].y - p[i].y;

#if SIM_USE_EUCLIDEAN_SEPARATION

            int32_t distance_sq = (int32_t)dx * dx + (int32_t)dy * dy;

            if (distance_sq >= SIM_PARTICLE_MIN_DISTANCE_SQ)
                continue;

            // Calculate the Euclidean distance.
            uint16_t distance = isqrt_u32((uint32_t)distance_sq);

            // Avoid division by zero when particles occupy the same position.
            if (distance == 0)
            {
                dx = 1;
                dy = 0;
                distance = 1;
            }

            // Calculate the overlap along the normalized direction vector.
            uint16_t overlap = SIM_PARTICLE_MIN_DISTANCE - distance;

            int16_t correction_x = (int16_t)((int32_t)dx * overlap / (2 * distance));
            int16_t correction_y = (int16_t)((int32_t)dy * overlap / (2 * distance));

            // Apply equal and opposite corrections to both particles.
            p[i].x = sat_add_u8_s8(p[i].x, (int8_t)-correction_x);
            p[i].y = sat_add_u8_s8(p[i].y, (int8_t)-correction_y);

            p[j].x = sat_add_u8_s8(p[j].x, (int8_t)correction_x);
            p[j].y = sat_add_u8_s8(p[j].y, (int8_t)correction_y);

#else // Use axis-aligned separation

            uint8_t ax = dx > 0 ? dx : -dx;
            uint8_t ay = dy > 0 ? dy : -dy;

            // Already separated along at least one axis
            if (ax >= SIM_PARTICLE_MIN_DISTANCE ||
                ay >= SIM_PARTICLE_MIN_DISTANCE)
                continue;

            uint8_t overlap;
            int8_t correction_i;
            int8_t correction_j;

            if (ax >= ay)
            {
                overlap = SIM_PARTICLE_MIN_DISTANCE - ax;

                correction_i = overlap >> 1; // Half of the overlap
                correction_j = overlap - correction_i;

                correction_i = dx > 0 ? -correction_i : correction_i;
                correction_j = dx >= 0 ? correction_j : -correction_j;

                p[i].x = sat_add_u8_s8(p[i].x, correction_i);
                p[j].x = sat_add_u8_s8(p[j].x, correction_j);
            }
            else
            {
                overlap = SIM_PARTICLE_MIN_DISTANCE - ay;

                correction_i = overlap >> 1;
                correction_j = overlap - correction_i;

                correction_i = dy > 0 ? -correction_i : correction_i;
                correction_j = dy >= 0 ? correction_j : -correction_j;

                p[i].y = sat_add_u8_s8(p[i].y, correction_i);
                p[j].y = sat_add_u8_s8(p[j].y, correction_j);
            }

#endif // SIM_USE_EUCLIDEAN_SEPARATION
        }
    }

    // Calculate COM
    if (frame_counter % SIM_CENTER_OF_MASS_INTERVAL == SIM_CENTER_OF_MASS_REMAINDER)
    {
        cum_com.x = 0u;
        cum_com.y = 0u;

        for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
        {
            cum_com.x += p[i].x;
            cum_com.y += p[i].y;
        }

        uint8_t com_x = cum_com.x / SIM_PARTICLE_COUNT;
        uint8_t com_y = cum_com.y / SIM_PARTICLE_COUNT;

        // Move towards COM by one unit
        for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
        {
            if (com_x > p[i].x)
                ++p[i].x;
            else if (com_x < p[i].x)
                --p[i].x;

            if (com_y > p[i].y)
                ++p[i].y;
            else if (com_y < p[i].y)
                --p[i].y;
        }
    }

    // Apply friction (experimental)
    if (frame_counter % SIM_FRICTION_INTERVAL == SIM_FRICTION_REMAINDER)
    {
        for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
        {
            // Reduce velocity by 1 unit, preserving direction.
            if (p[i].x > pp[i].x)
                ++pp[i].x;
            else if (p[i].x < pp[i].x)
                --pp[i].x;

            if (p[i].y > pp[i].y)
                ++pp[i].y;
            else if (p[i].y < pp[i].y)
                --pp[i].y;
        }
    }
}
