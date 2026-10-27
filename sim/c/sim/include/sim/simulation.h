#ifndef SIMULATION_H
#define SIMULATION_H

#include <stdint.h>

#include "config.h"
#include "vec2.h"

#ifdef __cplusplus
extern "C" {
#endif


extern vec2u p[SIM_PARTICLE_COUNT];     // current position
extern vec2u pp[SIM_PARTICLE_COUNT];    // previous position
extern vec2i a;                         // current acceleration

void simulate_frame(void);


#ifdef __cplusplus
}
#endif

#endif // SIMULATION_H
