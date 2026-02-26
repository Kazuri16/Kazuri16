#include "log_manager.h"

#include <stdio.h>

static log_config_t g_cfg;

static void rollover_if_needed(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return;
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fclose(f);

    if (size > 0 && (size_t)size >= g_cfg.rollover_bytes) {
        char backup[128];
        snprintf(backup, sizeof(backup), "%s.1", path);
        remove(backup);
        rename(path, backup);
    }
}

bool log_init(const log_config_t *cfg) {
    if (!cfg || !cfg->raw_path || !cfg->state_path || !cfg->anomaly_path) {
        return false;
    }
    g_cfg = *cfg;
    rollover_if_needed(g_cfg.raw_path);
    rollover_if_needed(g_cfg.state_path);
    rollover_if_needed(g_cfg.anomaly_path);
    return true;
}

bool log_raw_frame(const can_frame_t *frame) {
    FILE *f = fopen(g_cfg.raw_path, "a");
    if (!f) {
        return false;
    }
    fprintf(f, "%u,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
            frame->timestamp_ms,
            (unsigned long)frame->can_id,
            frame->dlc,
            frame->data[0], frame->data[1], frame->data[2], frame->data[3],
            frame->data[4], frame->data[5], frame->data[6], frame->data[7]);
    fclose(f);
    return true;
}

bool log_state(const vehicle_state_t *state) {
    FILE *f = fopen(g_cfg.state_path, "a");
    if (!f) {
        return false;
    }
    fprintf(f, "%u,%.2f,%.2f,%d,%.2f,%u,%u,%u\n",
            state->timestamp_ms,
            state->speed_kph,
            state->rpm,
            state->gear,
            state->throttle_pct,
            state->engine_on,
            state->brake_on,
            state->fault_on);
    fclose(f);
    return true;
}

bool log_anomaly(const anomaly_event_t *event) {
    FILE *f = fopen(g_cfg.anomaly_path, "a");
    if (!f) {
        return false;
    }
    fprintf(f, "%u,%u,%s,%.6f,%.6f,%.2f,%.2f\n",
            event->timestamp_ms,
            event->event_id,
            event->description,
            event->gps_snapshot.latitude,
            event->gps_snapshot.longitude,
            event->state_snapshot.speed_kph,
            event->state_snapshot.rpm);
    fclose(f);
    return true;
}

void log_flush(void) {
    // Placeholder for buffered IO implementation on target filesystem.
}
