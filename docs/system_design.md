# Ultra-Low-Power Intelligent CAN Bus Data Logger - Embedded Design

## 1. Target Platform
- **MCU options**: ESP32 (ESP-IDF) or STM32L4 (HAL/LL).
- **External peripherals**:
  - MCP2515 over SPI (CAN controller)
  - SN65HVD230/TJA1050 (CAN transceiver)
  - NEO-6M over UART (GPS)
  - microSD over SPI/SDIO (FAT32)

## 2. Layered Architecture

### Hardware Layer
- `mcp2515_driver`: configures CAN bitrate and listen-only mode.
- `gps_uart_driver`: receives NMEA stream.
- `sd_driver`: append-only line logging with explicit flush.
- `power_manager`: deep sleep transitions and wake handling.

### Middleware Layer
- `can_abstraction`: normalizes standard/extended CAN frames.
- `gps_parser`: decodes GPRMC/GPGGA fields.
- `log_manager`: manages rolling files and anomaly/event files.

### Application Layer
- `vehicle_state_engine`: reconstructs speed, RPM, throttle, gear, brake, fault and engine status.
- `anomaly_engine`: rule-based and statistical anomaly checks.

## 3. Runtime Data Flow
1. CAN IRQ signals MCP2515 RX buffer ready.
2. Firmware reads frame from MCP2515 (timestamped in software ticks).
3. Frame written to raw log buffer.
4. Vehicle state engine updates fields for known IDs.
5. Anomaly engine checks threshold/range/missing-frame/statistical rules.
6. If anomaly detected:
   - add event into anomaly buffer,
   - snapshot current vehicle state,
   - attach latest GPS position,
   - toggle LED indicator.
7. Log manager batches SD writes and performs rollover at size limit.

## 4. Power Strategy
- **Active mode**: normal capture + decode + anomaly checks.
- **Reduced-frequency mode**: lower CPU clock when frame interval exceeds idle threshold.
- **Deep sleep mode**: no CAN activity for configurable timeout; wake on CAN IRQ GPIO.

## 5. Reliability and Safety
- Watchdog kicks in main loop and worker tasks.
- Startup integrity checks and fallback-safe defaults.
- Listen-only MCP2515 configuration enforced on every boot.
- No CAN TX paths in firmware build.

## 6. Test Strategy
- Virtual frame feed for deterministic unit tests.
- Burst-load synthetic CAN traffic for logger throughput.
- Missing periodic frames, RPM spikes and invalid value injections.
- GPS lock/loss scenarios for fallback coordinates handling.
- Long-run stress test with periodic forced resets.
- Host simulation harness (`firmware/tests/simulate_logger.c`) to replay synthetic CAN+GPS sequences and verify anomaly outcomes before target flashing.
