# PerPompa v1.06 — Modbus RTU + I²C LCD

**Status: DEVELOPMENT ONLY / NOT FLASHABLE.** v1.06 is a new user-requested version. No integrated Arduino sketch or firmware binary exists here yet; the checked-in RTU frame receiver is **not** a usable pump control program.

## Goals
- Keep approved v1.05 and archived v1.04 intact; no motor/STEP/RNG/EEPROM/STOP guard regressions.
- Replace application USB control with Modbus RTU Slave over TTL-to-RS485 via UART D0/D1. Keep UART bootloader for firmware uploads; ensure RS485 hardware cannot interfere with upload.
- Convert existing HD44780 16x2 LCD to an I²C PCF8574 backpack on A4/A5; preserve encoder and current menu.
- PLC can configure volume, speed, mode, direction, display units, Remote D5 settings and read commanded motor state, fault and estimated steps; critical hardware setup and calibration require independent local confirmation.
- Future Windows PC program uses **the same register map**; only one Modbus master at a time.

## Technical locks
- Only firmware change after complete real-source positive/negative testing, combined regression, **real AVR 8 MHz and 16 MHz builds**, source/blob integrity verification, and required approvals.
- Root AGENTS.md and v1.05 REVISION_MANIFEST.md still identify v1.05 as the active version. This v1.06 folder is **draft**; do not silently rewrite locked policies or change active deployment.
- No Actions, force-push, Work/Codex. All Git commits [skip ci], main only, expected HEAD required.
- No physical acceptance, no industrial safety claim: HW NOT_RUN / RELEASE_BLOCKED.

## Prototype frame receiver
modbus_rtu_frame.h has no Arduino/GPIO/motor dependencies and uses actual per-byte timestamps from an external receive layer. **The existing blocking step generator does not supply accurate UART arrival times by polling Serial.available().** Scheduling/receiving strategy must be verified before integration.

Standalone test: g++ -std=c++11 -Wall -Wextra -Werror -pedantic -fsanitize=undefined tests/modbus_rtu_frame_native.cpp -o /tmp/perpompa_rtu && /tmp/perpompa_rtu

See HARDWARE.md and MODBUS_REGISTER_MAP.md.

Latest scoped test: 101/101 host checks with g++ and clang++ at F_CPU=8/16MHz (F_CPU is only a build macro; NOT an AVR compile). Firmware integration NOT_RUN.
