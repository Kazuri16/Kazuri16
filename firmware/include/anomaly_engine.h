#ifndef ANOMALY_ENGINE_H
#define ANOMALY_ENGINE_H

#include "logger_types.h"

typedef struct {
    float speed_limit_kph;
    float max_rpm_delta_per_s;
    uint32_t periodic_timeout_ms;
    uint32_t expected_periodic_id;
    uint32_t known_ids[32];
    uint8_t known_id_count;
} anomaly_config_t;

typedef struct {
    float rpm_window[16];
    uint8_t rpm_count;
    uint8_t rpm_index;
    float last_rpm;
    uint32_t last_rpm_ts;
    uint32_t last_periodic_ts;
    uint32_t event_counter;
} anomaly_runtime_t;

void anomaly_init(anomaly_runtime_t *runtime);
bool anomaly_check(
    const anomaly_config_t *cfg,
    anomaly_runtime_t *runtime,
    const can_frame_t *frame,
    const vehicle_state_t *state,
    const gps_fix_t *gps,
    anomaly_event_t *event_out);

#endif
