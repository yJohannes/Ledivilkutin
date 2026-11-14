/// @file sim_config.h
/// @author Johannes Ylinen
/// @brief Simulation parameters, constant values, etc.


#ifndef SIM_CONFIG_H
#define SIM_CONFIG_H

#include <stdint.h>


// Constant parameters
#define LEDS_X      16u
#define LEDS_Y      16u
#define LED_COUNT   256u  // LEDS_X * LEDS_Y

#define SIM_X_MIN   0u
#define SIM_Y_MIN   0u
#define SIM_X_MAX   UINT8_MAX
#define SIM_Y_MAX   UINT8_MAX

#define SIM_WIDTH   UINT8_VALUE_COUNT
#define SIM_HEIGHT  UINT8_VALUE_COUNT
#define SIM_CENTER_X    128
#define SIM_CENTER_Y    128
#define SIM_CELL_WIDTH      16u  // SIM_WIDTH / LEDS_X
#define SIM_CELL_HEIGHT     16u  // SIM_HEIGHT / LEDS_Y

// Constant as of now
#define SIM_DEFAULT_PARTICLE_COUNT      73u // 75 -> 73 because of the weirdest bug I've ever come across my whole entire day and ICBA to spend another indeterminate amount of time trying to hunt it down since it's 3 AM as I'm typing this proper length comment out to remember this devious, diabolical bug spawned by the devil himself where three (3) particles get stuck on y=0 and move freely on the x-axis and this didn't occur before I refactored code to modules and the three-state state machine so therefore I must conclude good programming practices are the root of all evil, not premature optimizations as someone in history might have said just like I'm saying something on line 30, now at column nr. 702, on sim_config.h at 3.17 AM on the 14th of November, 2026 while questioning my life choices and thinking what even is programming? End of sentence. Nobody will ever read this so maybe I should confess something extremely embarassing. Scrap that I will instead express my frustration for university programming courses. "Lines should be kept under 80 characters, do this... do that..." RAHH YOU CAN'T TELL ME WHAT TO DO I'M FREEEEEEEEEEEE UNLIKE YOUUUUUUUUUUUUUU THIS IS MY CODE I DO WHATEVER I FEEL LIKE DOING AND YOU CAN'T DO A_N_Y_T_H_I_N_G ABOUT IT >:]. THIS IS WHAT IT'S ALL ABOUT writing a 1300+ letter long comment. Good night.
#define SIM_PARTICLE_COUNT              SIM_DEFAULT_PARTICLE_COUNT

// Tweakable parameters
#define SIM_DEFAULT_FPS                         20u
#define SIM_DEFAULT_PARTICLE_RADIUS             32u
#define SIM_DEFAULT_PARTICLE_MIN_DISTANCE       16   // Distance before particles are pushed apart
#define SIM_DEFAULT_PARTICLE_MAX_DISPLACEMENT   16   // Maximum dx in Verlet integration
#define SIM_DEFAULT_ACCELERATION_INTERVAL       3u
#define SIM_DEFAULT_ACCELERATION_REMAINDER      1u
#define SIM_DEFAULT_CENTER_OF_MASS_INTERVAL     3u
#define SIM_DEFAULT_CENTER_OF_MASS_REMAINDER    0u
#define SIM_DEFAULT_FRICTION_INTERVAL           5u
#define SIM_DEFAULT_FRICTION_REMAINDER          4u

#ifndef SIM_USE_DYNAMIC

// Defaults for non-dynamic
#define SIM_FPS                         SIM_DEFAULT_FPS
#define SIM_PARTICLE_RADIUS             SIM_DEFAULT_PARTICLE_RADIUS
#define SIM_PARTICLE_MIN_DISTANCE       SIM_DEFAULT_PARTICLE_MIN_DISTANCE
#define SIM_PARTICLE_MAX_DISPLACEMENT   SIM_DEFAULT_PARTICLE_MAX_DISPLACEMENT
#define SIM_ACCELERATION_INTERVAL       SIM_DEFAULT_ACCELERATION_INTERVAL
#define SIM_ACCELERATION_REMAINDER      SIM_DEFAULT_ACCELERATION_REMAINDER
#define SIM_CENTER_OF_MASS_INTERVAL     SIM_DEFAULT_CENTER_OF_MASS_INTERVAL
#define SIM_CENTER_OF_MASS_REMAINDER    SIM_DEFAULT_CENTER_OF_MASS_REMAINDER
#define SIM_FRICTION_INTERVAL           SIM_DEFAULT_FRICTION_INTERVAL
#define SIM_FRICTION_REMAINDER          SIM_DEFAULT_FRICTION_REMAINDER

#else

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    uint8_t fps;
    uint8_t particle_radius;
    uint8_t particle_min_distance;
    uint8_t particle_max_displacement;
    uint8_t acceleration_interval;
    uint8_t acceleration_remainder;
    uint8_t center_of_mass_interval;
    uint8_t center_of_mass_remainder;
    uint8_t friction_interval;
    uint8_t friction_remainder;
} sim_config;


extern sim_config cfg;


#ifdef __cplusplus
}
#endif

#define SIM_FPS                         cfg.fps
#define SIM_PARTICLE_RADIUS             cfg.particle_radius
#define SIM_PARTICLE_MIN_DISTANCE       cfg.particle_min_distance
#define SIM_PARTICLE_MAX_DISPLACEMENT   cfg.particle_max_displacement
#define SIM_ACCELERATION_INTERVAL       cfg.acceleration_interval
#define SIM_ACCELERATION_REMAINDER      cfg.acceleration_remainder
#define SIM_CENTER_OF_MASS_INTERVAL     cfg.center_of_mass_interval
#define SIM_CENTER_OF_MASS_REMAINDER    cfg.center_of_mass_remainder
#define SIM_FRICTION_INTERVAL           cfg.friction_interval
#define SIM_FRICTION_REMAINDER          cfg.friction_remainder

#endif

#endif // SIM_CONFIG_H
