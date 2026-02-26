#ifndef VEHICLE_STATE_ENGINE_H
#define VEHICLE_STATE_ENGINE_H

#include "logger_types.h"

void vehicle_state_init(vehicle_state_t *state);
void vehicle_state_update(vehicle_state_t *state, const can_frame_t *frame);

#endif
