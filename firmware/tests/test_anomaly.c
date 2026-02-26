#include <assert.h>

#include "anomaly_engine.h"

int main(void) {
    anomaly_config_t cfg = {
        .speed_limit_kph = 100.0f,
        .max_rpm_delta_per_s = 3000.0f,
        .periodic_timeout_ms = 1000,
        .expected_periodic_id = 0x0C0,
        .known_ids = {0x0C0, 0x0D0, 0x1A0},
        .known_id_count = 3,
    };

    anomaly_runtime_t runtime;
    anomaly_init(&runtime);

    can_frame_t frame = {.timestamp_ms = 100, .can_id = 0x0D0, .dlc = 8};
    vehicle_state_t state = {.speed_kph = 120.0f, .rpm = 1200.0f, .engine_on = true, .timestamp_ms = 100};
    gps_fix_t gps = {.fix_valid = true, .latitude = 1.0, .longitude = 2.0};
    anomaly_event_t event = {0};

    bool found = anomaly_check(&cfg, &runtime, &frame, &state, &gps, &event);
    assert(found);
    assert(event.type == ANOMALY_SPEED_THRESHOLD);

    return 0;
}
