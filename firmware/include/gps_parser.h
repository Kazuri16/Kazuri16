#ifndef GPS_PARSER_H
#define GPS_PARSER_H

#include <stdbool.h>
#include "logger_types.h"

bool gps_parse_sentence(const char *nmea, uint32_t timestamp_ms, gps_fix_t *fix);

#endif
