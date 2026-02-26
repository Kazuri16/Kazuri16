#include <stdbool.h>
#include <stdint.h>

#include "anomaly_engine.h"
#include "gps_parser.h"
#include "log_manager.h"
#include "vehicle_state_engine.h"

static bool can_receive_frame(can_frame_t *frame);
static bool gps_receive_line(char *line, uint32_t line_size);
static void watchdog_kick(void);
static void led_signal_anomaly(void);
static void low_power_manage(uint32_t inactivity_ms);

int main(void) {
    vehicle_state_t state;
    vehicle_state_init(&state);

    gps_fix_t gps = {0};
    anomaly_config_t cfg = {
        .speed_limit_kph = 140.0f,
        .max_rpm_delta_per_s = 2200.0f,
        .periodic_timeout_ms = 500,
        .expected_periodic_id = 0x0C0,
        .known_ids = {0x0C0, 0x0D0, 0x1A0},
        .known_id_count = 3,
    };

    anomaly_runtime_t runtime;
    anomaly_init(&runtime);

    log_config_t log_cfg = {
        .raw_path = "raw_can.csv",
        .state_path = "vehicle_state.csv",
        .anomaly_path = "anomaly.csv",
        .rollover_bytes = 1024UL * 1024UL,
    };
    log_init(&log_cfg);

    uint32_t last_frame_ms = 0;

    while (1) {
        can_frame_t frame;
        if (can_receive_frame(&frame)) {
            last_frame_ms = frame.timestamp_ms;
            vehicle_state_update(&state, &frame);
            log_raw_frame(&frame);
            log_state(&state);

            anomaly_event_t event;
            if (anomaly_check(&cfg, &runtime, &frame, &state, &gps, &event)) {
                log_anomaly(&event);
                led_signal_anomaly();
            }
        }

        char nmea[128];
        if (gps_receive_line(nmea, sizeof(nmea))) {
            gps_parse_sentence(nmea, state.timestamp_ms, &gps);
        }

        low_power_manage(state.timestamp_ms - last_frame_ms);
        watchdog_kick();
    }

    return 0;
}

static bool can_receive_frame(can_frame_t *frame) {
    (void)frame;
    return false;
}

static bool gps_receive_line(char *line, uint32_t line_size) {
    (void)line;
    (void)line_size;
    return false;
}

static void watchdog_kick(void) {}

static void led_signal_anomaly(void) {}

static void low_power_manage(uint32_t inactivity_ms) {
    (void)inactivity_ms;
}
