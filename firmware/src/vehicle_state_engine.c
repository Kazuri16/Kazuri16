#include "vehicle_state_engine.h"

#define CAN_ID_SPEED 0x0D0
#define CAN_ID_ENGINE 0x0C0
#define CAN_ID_BRAKE_GEAR 0x1A0

void vehicle_state_init(vehicle_state_t *state) {
    *state = (vehicle_state_t){0};
    state->gear = -1;
}

void vehicle_state_update(vehicle_state_t *state, const can_frame_t *frame) {
    if (!state || !frame) {
        return;
    }

    switch (frame->can_id) {
        case CAN_ID_SPEED:
            state->speed_kph = (float)frame->data[0];
            state->throttle_pct = ((float)frame->data[1] * 100.0f) / 255.0f;
            break;
        case CAN_ID_ENGINE: {
            uint16_t raw_rpm = (uint16_t)((frame->data[0] << 8U) | frame->data[1]);
            state->rpm = raw_rpm / 4.0f;
            state->engine_on = frame->data[2] & 0x01U;
            state->fault_on = frame->data[2] & 0x02U;
            break;
        }
        case CAN_ID_BRAKE_GEAR:
            state->gear = (int8_t)frame->data[0];
            state->brake_on = frame->data[1] & 0x01U;
            break;
        default:
            break;
    }

    state->timestamp_ms = frame->timestamp_ms;
}
