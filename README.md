# Ultra-Low-Power Intelligent CAN Bus Data Logger

Embedded-only firmware architecture and implementation scaffold for a passive CAN monitoring data logger with anomaly detection, GPS tagging, and low-power modes.

## Repository Layout
- `docs/system_design.md`: architecture, power strategy, safety and test approach.
- `firmware/include`: portable interface headers for state engine, anomaly engine, logging, and GPS parser.
- `firmware/src`: C implementations and firmware main loop scaffold.
- `firmware/tests`: host-side unit tests for core detection logic.

## Key Implemented Features
- Passive CAN frame structure supporting 11-bit and 29-bit IDs.
- Vehicle state reconstruction hooks for speed, RPM, throttle, gear, brake and fault status.
- Rule-based + rolling-statistical anomaly detection.
- GPS NMEA parsing (RMC) for event coordinate tagging.
- Rolling log manager interface for raw/state/anomaly logs.
- Main-loop skeleton with low-power and watchdog integration points.

## Build test (host)
```bash
gcc -std=c11 -Wall -Wextra -Ifirmware/include \
  firmware/src/anomaly_engine.c firmware/tests/test_anomaly.c -lm -o /tmp/test_anomaly && /tmp/test_anomaly
```
