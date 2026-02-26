#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <stdbool.h>
#include <stddef.h>
#include "logger_types.h"

typedef struct {
    const char *raw_path;
    const char *state_path;
    const char *anomaly_path;
    size_t rollover_bytes;
} log_config_t;

bool log_init(const log_config_t *cfg);
bool log_raw_frame(const can_frame_t *frame);
bool log_state(const vehicle_state_t *state);
bool log_anomaly(const anomaly_event_t *event);
void log_flush(void);

#endif
