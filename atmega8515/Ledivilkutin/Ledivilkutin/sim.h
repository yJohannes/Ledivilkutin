#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdint.h>

#include "sim_config.h"
#include "vec2.h"

#ifdef __cplusplus
extern "C" {
#endif


extern vec2u p[SIM_PARTICLE_COUNT];  // current position
extern vec2u pp[SIM_PARTICLE_COUNT]; // previous position
extern vec2i a;                      // current acceleration
extern vec2u16 sim_cum_com;          // Cumulative center of mass
extern uint8_t sim_accel_timer;      // When 0 applies acceleration on sim step.

void simulation_init(void);
void simulation_step(void);


#ifdef __cplusplus
}
#endif

#endif // SIMULATION_H
