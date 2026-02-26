#ifndef LOGGER_TYPES_H
#define LOGGER_TYPES_H

#include <stdbool.h>
#include <stdint.h>

#define CAN_MAX_DATA_LEN 8

typedef struct {
    uint32_t timestamp_ms;
    uint32_t can_id;
    uint8_t dlc;
    bool is_extended;
    uint8_t data[CAN_MAX_DATA_LEN];
} can_frame_t;

typedef struct {
    float speed_kph;
    float rpm;
    int8_t gear;
    float throttle_pct;
    bool engine_on;
    bool brake_on;
    bool fault_on;
    uint32_t timestamp_ms;
} vehicle_state_t;

typedef struct {
    double latitude;
    double longitude;
    float speed_knots;
    bool fix_valid;
    uint32_t timestamp_ms;
} gps_fix_t;

typedef enum {
    ANOMALY_NONE = 0,
    ANOMALY_SPEED_THRESHOLD,
    ANOMALY_RPM_SPIKE,
    ANOMALY_MISSING_PERIODIC,
    ANOMALY_UNKNOWN_ID,
    ANOMALY_ENGINE_OFF_RPM,
    ANOMALY_INVALID_RANGE,
    ANOMALY_SIGMA_DEVIATION,
    ANOMALY_SIGNAL_JUMP
} anomaly_type_t;

typedef struct {
    uint32_t event_id;
    anomaly_type_t type;
    const char *description;
    vehicle_state_t state_snapshot;
    gps_fix_t gps_snapshot;
    uint32_t timestamp_ms;
} anomaly_event_t;

#endif
