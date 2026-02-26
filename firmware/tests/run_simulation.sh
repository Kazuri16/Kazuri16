#!/usr/bin/env bash
set -euo pipefail

gcc -std=c11 -Wall -Wextra -Ifirmware/include \
  firmware/src/anomaly_engine.c \
  firmware/src/vehicle_state_engine.c \
  firmware/src/gps_parser.c \
  firmware/tests/simulate_logger.c \
  -lm -o /tmp/simulate_logger

/tmp/simulate_logger
