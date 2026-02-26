#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "anomaly_engine.h"
#include "gps_parser.h"
#include "vehicle_state_engine.h"

static can_frame_t mk_frame(uint32_t ts, uint32_t id, uint8_t b0, uint8_t b1, uint8_t b2) {
    can_frame_t f = {0};
    f.timestamp_ms = ts;
    f.can_id = id;
    f.dlc = 8;
    f.data[0] = b0;
    f.data[1] = b1;
    f.data[2] = b2;
    return f;
}

int main(void) {
    vehicle_state_t state;
    vehicle_state_init(&state);

    gps_fix_t gps = {0};
    bool parsed = gps_parse_sentence(
        "$GPRMC,151229.00,A,3723.2475,N,12158.3416,W,023.4,84.4,230394,003.1,W*6A",
        100,
        &gps);
    assert(parsed);
    assert(gps.fix_valid);

    anomaly_config_t cfg = {
        .speed_limit_kph = 100.0f,
        .max_rpm_delta_per_s = 2000.0f,
        .periodic_timeout_ms = 400,
        .expected_periodic_id = 0x0C0,
        .known_ids = {0x0C0, 0x0D0, 0x1A0},
        .known_id_count = 3,
    };

    anomaly_runtime_t runtime;
    anomaly_init(&runtime);

    anomaly_event_t event = {0};
    uint32_t anomaly_count = 0;
    bool saw_speed = false;
    bool saw_unknown = false;
    bool saw_rpm_spike = false;

    can_frame_t traffic[] = {
        mk_frame(100, 0x0C0, 0x1F, 0x40, 0x01), // 2000 rpm, engine on
        mk_frame(130, 0x0D0, 50, 100, 0),       // 50 kph
        mk_frame(150, 0x1A0, 3, 0, 0),          // gear/brake
        mk_frame(260, 0x0D0, 145, 120, 0),      // speed anomaly
        mk_frame(280, 0x0D0, 70, 110, 0),       // speed back to valid range
        mk_frame(300, 0x777, 1, 2, 3),          // unknown id anomaly
        mk_frame(520, 0x0C0, 0x40, 0x00, 0x01), // rpm spike anomaly
    };

    const size_t frame_count = sizeof(traffic) / sizeof(traffic[0]);

    for (size_t i = 0; i < frame_count; i++) {
        vehicle_state_update(&state, &traffic[i]);
        if (anomaly_check(&cfg, &runtime, &traffic[i], &state, &gps, &event)) {
            anomaly_count++;
            saw_speed |= (event.type == ANOMALY_SPEED_THRESHOLD);
            saw_unknown |= (event.type == ANOMALY_UNKNOWN_ID);
            saw_rpm_spike |= (event.type == ANOMALY_RPM_SPIKE);
            printf("anomaly[%u]: type=%d desc=%s ts=%u speed=%.1f rpm=%.1f\n",
                   event.event_id,
                   event.type,
                   event.description,
                   event.timestamp_ms,
                   event.state_snapshot.speed_kph,
                   event.state_snapshot.rpm);
        }
    }

    assert(anomaly_count >= 3);
    assert(saw_speed);
    assert(saw_unknown);
    assert(saw_rpm_spike);
    assert(state.speed_kph == 70.0f);
    assert(state.rpm > 0.0f);

    puts("simulation_pass");
    return 0;
}
