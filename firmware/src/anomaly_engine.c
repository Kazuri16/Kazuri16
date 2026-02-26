#include "anomaly_engine.h"

#include <math.h>
#include <stddef.h>
#include <string.h>

static bool is_known_id(const anomaly_config_t *cfg, uint32_t id) {
    for (uint8_t i = 0; i < cfg->known_id_count; i++) {
        if (cfg->known_ids[i] == id) {
            return true;
        }
    }
    return false;
}

static void populate_event(
    anomaly_runtime_t *runtime,
    anomaly_type_t type,
    const char *desc,
    const vehicle_state_t *state,
    const gps_fix_t *gps,
    anomaly_event_t *event_out) {
    runtime->event_counter++;
    event_out->event_id = runtime->event_counter;
    event_out->type = type;
    event_out->description = desc;
    event_out->state_snapshot = *state;
    event_out->gps_snapshot = *gps;
    event_out->timestamp_ms = state->timestamp_ms;
}

static bool sigma_check(anomaly_runtime_t *runtime, float sample) {
    runtime->rpm_window[runtime->rpm_index] = sample;
    runtime->rpm_index = (runtime->rpm_index + 1U) % 16U;
    if (runtime->rpm_count < 16U) {
        runtime->rpm_count++;
        return false;
    }

    float mean = 0.0f;
    for (uint8_t i = 0; i < runtime->rpm_count; i++) {
        mean += runtime->rpm_window[i];
    }
    mean /= runtime->rpm_count;

    float variance = 0.0f;
    for (uint8_t i = 0; i < runtime->rpm_count; i++) {
        float d = runtime->rpm_window[i] - mean;
        variance += d * d;
    }
    variance /= runtime->rpm_count;

    float sigma = sqrtf(variance);
    return fabsf(sample - mean) > (3.0f * sigma);
}

void anomaly_init(anomaly_runtime_t *runtime) {
    memset(runtime, 0, sizeof(*runtime));
}

bool anomaly_check(
    const anomaly_config_t *cfg,
    anomaly_runtime_t *runtime,
    const can_frame_t *frame,
    const vehicle_state_t *state,
    const gps_fix_t *gps,
    anomaly_event_t *event_out) {
    if (!cfg || !runtime || !frame || !state || !gps || !event_out) {
        return false;
    }

    if (state->speed_kph > cfg->speed_limit_kph) {
        populate_event(runtime, ANOMALY_SPEED_THRESHOLD, "Speed over configured threshold", state, gps, event_out);
        return true;
    }

    if (!is_known_id(cfg, frame->can_id)) {
        populate_event(runtime, ANOMALY_UNKNOWN_ID, "Unexpected CAN ID observed", state, gps, event_out);
        return true;
    }

    if (!state->engine_on && state->rpm > 0.0f) {
        populate_event(runtime, ANOMALY_ENGINE_OFF_RPM, "Engine OFF state with non-zero RPM", state, gps, event_out);
        return true;
    }

    if (state->rpm < 0.0f || state->throttle_pct < 0.0f || state->throttle_pct > 100.0f) {
        populate_event(runtime, ANOMALY_INVALID_RANGE, "Invalid reconstructed vehicle parameter range", state, gps, event_out);
        return true;
    }

    if (runtime->last_rpm_ts > 0U) {
        uint32_t dt_ms = state->timestamp_ms - runtime->last_rpm_ts;
        if (dt_ms > 0U) {
            float delta_per_s = fabsf(state->rpm - runtime->last_rpm) * 1000.0f / dt_ms;
            if (delta_per_s > cfg->max_rpm_delta_per_s) {
                populate_event(runtime, ANOMALY_RPM_SPIKE, "RPM spike above configured delta/s", state, gps, event_out);
                runtime->last_rpm = state->rpm;
                runtime->last_rpm_ts = state->timestamp_ms;
                return true;
            }
        }
    }

    if (sigma_check(runtime, state->rpm)) {
        populate_event(runtime, ANOMALY_SIGMA_DEVIATION, "RPM 3-sigma deviation detected", state, gps, event_out);
        runtime->last_rpm = state->rpm;
        runtime->last_rpm_ts = state->timestamp_ms;
        return true;
    }

    if (cfg->expected_periodic_id == frame->can_id) {
        runtime->last_periodic_ts = state->timestamp_ms;
    } else if (runtime->last_periodic_ts > 0U &&
               (state->timestamp_ms - runtime->last_periodic_ts) > cfg->periodic_timeout_ms) {
        populate_event(runtime, ANOMALY_MISSING_PERIODIC, "Periodic message timeout", state, gps, event_out);
        return true;
    }

    runtime->last_rpm = state->rpm;
    runtime->last_rpm_ts = state->timestamp_ms;
    return false;
}
