#include "config.h"


sim_config cfg = {
    .fps                        = SIM_DEFAULT_FPS,
    .particle_radius            = SIM_DEFAULT_PARTICLE_RADIUS,
    .particle_min_distance      = SIM_DEFAULT_PARTICLE_MIN_DISTANCE,
    .particle_max_displacement  = SIM_DEFAULT_PARTICLE_MAX_DISPLACEMENT,
    .acceleration_interval      = SIM_DEFAULT_ACCELERATION_INTERVAL,
    .acceleration_remainder     = SIM_DEFAULT_ACCELERATION_REMAINDER,
    .center_of_mass_interval    = SIM_DEFAULT_CENTER_OF_MASS_INTERVAL,
    .center_of_mass_remainder   = SIM_DEFAULT_CENTER_OF_MASS_REMAINDER,
    .friction_interval          = SIM_DEFAULT_FRICTION_INTERVAL,
    .friction_remainder         = SIM_DEFAULT_FRICTION_REMAINDER,
};
