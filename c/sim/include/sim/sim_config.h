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
#define SIM_DEFAULT_PARTICLE_COUNT      75u
#define SIM_PARTICLE_COUNT              SIM_DEFAULT_PARTICLE_COUNT

// Tweakable parameters
#define SIM_DEFAULT_FPS                         20u
#define SIM_DEFAULT_PARTICLE_RADIUS             32u
#define SIM_DEFAULT_PARTICLE_MIN_DISTANCE       16   // Distance before particles are pushed apart
#define SIM_DEFAULT_PARTICLE_MAX_DISPLACEMENT   15   // Maximum dx in Verlet integration
#define SIM_DEFAULT_ACCELERATION_INTERVAL       3u
#define SIM_DEFAULT_ACCELERATION_REMAINDER      0u
#define SIM_DEFAULT_CENTER_OF_MASS_INTERVAL     3u
#define SIM_DEFAULT_CENTER_OF_MASS_REMAINDER    0u
#define SIM_DEFAULT_FRICTION_INTERVAL           6u
#define SIM_DEFAULT_FRICTION_REMAINDER          5u

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
