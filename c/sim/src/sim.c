#include "sim.h"

#include <stdint.h>
#include <stdbool.h>

#include "sim_config.h"
#include "int_arithmetic.h"
#include "vec2.h"


// Global
vec2u p[SIM_PARTICLE_COUNT];
vec2u pp[SIM_PARTICLE_COUNT];
vec2i a;
vec2u16 sim_cum_com;
uint8_t sim_accel_timer;

static vec2u sim_prev_com; // not currently in use

static uint8_t sim_com_timer;
static uint8_t sim_friction_timer;


void simulation_init(void)
{
    sim_prev_com.x = 128u;
    sim_prev_com.y = 128u;

    sim_accel_timer = SIM_ACCELERATION_INTERVAL - SIM_ACCELERATION_REMAINDER;


    for (uint8_t i = 0; i < SIM_PARTICLE_COUNT; ++i)
    {
        int8_t x = (i % 9) - 4;
        int8_t y = (i / 9) - 4;

        p[i] = (vec2u)
        {
            SIM_CENTER_X + x,
            SIM_CENTER_Y + y
        };

        pp[i] = p[i];
    }
}


void simulation_step(void)
{
#ifdef SIM_INTEGRATION_ENABLED

#ifdef SIM_USE_POINTER_ITERATION
    vec2u* p_it = p;
    vec2u* pp_it = pp;

    for (uint8_t n = SIM_PARTICLE_COUNT; n; --n, ++p_it, ++pp_it)
    {
        uint8_t x = p_it->x;
        uint8_t y = p_it->y;

        int8_t dx = x - pp_it->x;
        int8_t dy = y - pp_it->y;

        if (dx > SIM_PARTICLE_MAX_DISPLACEMENT)       dx = SIM_PARTICLE_MAX_DISPLACEMENT;
        else if (dx < -SIM_PARTICLE_MAX_DISPLACEMENT) dx = -SIM_PARTICLE_MAX_DISPLACEMENT;

        if (dy > SIM_PARTICLE_MAX_DISPLACEMENT)       dy = SIM_PARTICLE_MAX_DISPLACEMENT;
        else if (dy < -SIM_PARTICLE_MAX_DISPLACEMENT) dy = -SIM_PARTICLE_MAX_DISPLACEMENT;

        pp_it->x = x;  // reuse locals, no reload
        pp_it->y = y;

        p_it->x = try_add_u8_s8(x, dx + a.x);
        p_it->y = try_add_u8_s8(y, dy + a.y);
    }
#else
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
        // Use try_add because it prevents weird border behaviour
        // Trust that dx + a.x won't over/underflow :] (it shouldn't)
        p[i].x = try_add_u8_s8(p[i].x, dx + a.x);
        p[i].y = try_add_u8_s8(p[i].y, dy + a.y);
    }
#endif // SIM_USE_POINTER_ITERATION
#endif // SIM_INTEGRATION_ENABLED

#ifdef SIM_COLLISIONS_ENABLED

    // for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT - 1; ++i)
    // {
    //     uint8_t xi = p[i].x;
    //     uint8_t yi = p[i].y;
    //     vec2u *pj = &p[i + 1];

    //     for (uint8_t n = SIM_PARTICLE_COUNT - 1 - i; n; --n, ++pj)
    //     {
    //         uint8_t xj = pj->x;

    //         // Exact 8-bit |dx|, no int16 widening
    //         bool right = xj >= xi;                 // original: dx >= 0
    //         uint8_t ax = right ? xj - xi : xi - xj;
    //         if (ax >= SIM_PARTICLE_MIN_DISTANCE)
    //             continue;                          // most pairs exit here

    //         uint8_t yj = pj->y;
    //         bool below = yj >= yi;                 // original: dy >= 0
    //         uint8_t ay = below ? yj - yi : yi - yj;
    //         if (ay >= SIM_PARTICLE_MIN_DISTANCE)
    //             continue;

    //         uint8_t overlap, ci, cj;

    //         if (ax >= ay)
    //         {
    //             overlap = SIM_PARTICLE_MIN_DISTANCE - ax;
    //             ci = overlap >> 1;
    //             cj = overlap - ci;
    //             xi    = try_add_u8_s8(xi,    right ? -(int8_t)ci : (int8_t)ci);
    //             pj->x = try_add_u8_s8(xj,    right ? (int8_t)cj  : -(int8_t)cj);
    //         }
    //         else
    //         {
    //             overlap = SIM_PARTICLE_MIN_DISTANCE - ay;
    //             ci = overlap >> 1;
    //             cj = overlap - ci;
    //             yi    = try_add_u8_s8(yi,    below ? -(int8_t)ci : (int8_t)ci);
    //             pj->y = try_add_u8_s8(yj,    below ? (int8_t)cj  : -(int8_t)cj);
    //         }
    //     }

    //     p[i].x = xi;
    //     p[i].y = yi;
    // }




    // Handle overlaps
    for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
    {
        // Handle each pair only
        for (uint8_t j = i + 1; j < SIM_PARTICLE_COUNT; ++j)
        {

            // // Bitwise | avoids a second branch
            // if ((ax >= SIM_PARTICLE_MIN_DISTANCE) | (ay >= SIM_PARTICLE_MIN_DISTANCE))
            //     continue;


            uint8_t ax = abs_diff_u8(p[j].x, p[i].x);
            uint8_t ay = abs_diff_u8(p[j].y, p[i].y);


            // Do axis-aligned separation
            // uint8_t ax = dx > 0 ? dx : -dx;
            // uint8_t ay = dy > 0 ? dy : -dy;

            // Already separated along at least one axis
            if (ax >= SIM_PARTICLE_MIN_DISTANCE ||
                ay >= SIM_PARTICLE_MIN_DISTANCE)
                continue;

            bool dx_negative = p[j].x < p[i].x;
            bool dy_negative = p[j].y < p[i].y;

            uint8_t overlap;
            int8_t correction_i;
            int8_t correction_j;

            if (ax >= ay)
            {
                overlap = SIM_PARTICLE_MIN_DISTANCE - ax;

                correction_i = overlap >> 1; // Half of the overlap
                correction_j = overlap - correction_i;

                // correction_i = dx < 0 ? correction_i : -correction_i;
                // correction_j = dx < 0 ? -correction_j : correction_j;


                correction_i = dx_negative ? correction_i : -correction_i;
                correction_j = dx_negative ? -correction_j : correction_j;

                // Use try_add because it prevents weird border behaviour
                p[i].x = try_add_u8_s8(p[i].x, correction_i);
                p[j].x = try_add_u8_s8(p[j].x, correction_j);
            }
            else
            {
                overlap = SIM_PARTICLE_MIN_DISTANCE - ay;

                correction_i = overlap >> 1;
                correction_j = overlap - correction_i;

                // correction_i = dy < 0 ? correction_i : -correction_i;
                // correction_j = dy < 0 ? -correction_j : correction_j;

                correction_i = dy_negative ? correction_i : -correction_i;
                correction_j = dy_negative ? -correction_j : correction_j;

                // Use try_add because it prevents weird border behaviour
                p[i].y = try_add_u8_s8(p[i].y, correction_i);
                p[j].y = try_add_u8_s8(p[j].y, correction_j);
            }
        }
    }
#endif // SIM_COLLISIONS_ENABLED


#ifdef SIM_COM_ENABLED

    // Calculate COM
    if (--sim_com_timer == 0)
    {
        sim_com_timer = SIM_CENTER_OF_MASS_INTERVAL;

        sim_cum_com.x = 0u;
        sim_cum_com.y = 0u;


#ifdef SIM_USE_POINTER_ITERATION
        p_it = p;

        for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i, ++p_it)
        {
            sim_cum_com.x += p_it->x;
            sim_cum_com.y += p_it->y;
        }
#else
        for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
        {
            sim_cum_com.x += p[i].x;
            sim_cum_com.y += p[i].y;
        }
#endif // SIM_USE_POINTER_ITERATION

        uint8_t com_x = sim_cum_com.x / SIM_PARTICLE_COUNT;
        uint8_t com_y = sim_cum_com.y / SIM_PARTICLE_COUNT;

        sim_prev_com.x = com_x;
        sim_prev_com.y = com_y;

        // Move towards COM by one unit
#ifdef SIM_USE_POINTER_ITERATION
        p_it = p;

        for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i, ++p_it)
        {
            if (com_x > p_it->x)
                ++p_it->x;
            else if (com_x < p_it->x)
                --p_it->x;

            if (com_y > p_it->y)
                ++p_it->y;
            else if (com_y < p_it->y)
                --p_it->y;
        }
#else
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
#endif // SIM_USE_POINTER_ITERATION
    }
#endif // SIM_COM_ENABLED


#ifdef SIM_FRICTION_ENABLED
    // Apply friction (experimental)


    if (--sim_friction_timer == 0)
    {
        sim_friction_timer = SIM_FRICTION_INTERVAL;

#ifdef SIM_USE_POINTER_ITERATION
        p_it = p;
        vec2u* pp_it = pp;
#endif // SIM_USE_POINTER_ITERATION
        for (uint8_t i = 0u; i < SIM_PARTICLE_COUNT; ++i)
        {
#ifdef SIM_USE_POINTER_ITERATION
            if (p_it->x > pp_it->x)
                ++pp_it->x;
            else if (p_it->x < pp_it->x)
                --pp_it->x;

            if (p_it->y > pp_it->y)
                ++pp_it->y;
            else if (p_it->y < pp_it->y)
                --pp_it->y;

            ++p_it;
            ++pp_it;
#else
            // Reduce velocity by 1 unit, preserving direction.
            if (p[i].x > pp[i].x)
                ++pp[i].x;
            else if (p[i].x < pp[i].x)
                --pp[i].x;

            if (p[i].y > pp[i].y)
                ++pp[i].y;
            else if (p[i].y < pp[i].y)
                --pp[i].y;
#endif // SIM_USE_POINTER_ITERATION
        }
    }
#endif // SIM_FRICTION_ENABLED
}
