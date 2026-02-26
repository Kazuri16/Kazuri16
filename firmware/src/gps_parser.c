#include "gps_parser.h"

#include <stdlib.h>
#include <string.h>

static double parse_degmin(const char *value) {
    if (!value || !*value) {
        return 0.0;
    }
    double raw = atof(value);
    int deg = (int)(raw / 100.0);
    double min = raw - (deg * 100.0);
    return deg + (min / 60.0);
}

bool gps_parse_sentence(const char *nmea, uint32_t timestamp_ms, gps_fix_t *fix) {
    if (!nmea || !fix) {
        return false;
    }

    if (strncmp(nmea, "$GPRMC", 6) != 0 && strncmp(nmea, "$GNRMC", 6) != 0) {
        return false;
    }

    char buf[128];
    strncpy(buf, nmea, sizeof(buf) - 1U);
    buf[sizeof(buf) - 1U] = '\0';

    char *tokens[16] = {0};
    uint8_t idx = 0;
    char *tok = strtok(buf, ",");
    while (tok && idx < 16U) {
        tokens[idx++] = tok;
        tok = strtok(NULL, ",");
    }

    if (idx < 8U) {
        return false;
    }

    fix->fix_valid = (tokens[2] && tokens[2][0] == 'A');
    if (!fix->fix_valid) {
        fix->timestamp_ms = timestamp_ms;
        return true;
    }

    double lat = parse_degmin(tokens[3]);
    if (tokens[4] && tokens[4][0] == 'S') {
        lat = -lat;
    }

    double lon = parse_degmin(tokens[5]);
    if (tokens[6] && tokens[6][0] == 'W') {
        lon = -lon;
    }

    fix->latitude = lat;
    fix->longitude = lon;
    fix->speed_knots = tokens[7] ? (float)atof(tokens[7]) : 0.0f;
    fix->timestamp_ms = timestamp_ms;
    return true;
}
