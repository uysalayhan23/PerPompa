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

## v1.06 Modbus Settings — first isolated implementation (2026-11-02)

The extra **Modbus Settings** main-menu page has an independently testable C++11 UI/state model, but is **not yet wired into the v1.06 Arduino sketch, I²C LCD or live UART**.

- `modbus_settings.h`: local encoder menu model: Baud Rate, Node Address, Parity, Stop Bits, Save + Apply (local HOLD confirmation), Back. Defaults **9600 / address 1 / EVEN / 1 stop**.
- EEPROM slots 192–207 and 224–239 store CRC16/CCITT-FALSE-verified, generation-checked, commit-last settings. Erased records display defaults; corrupt/conflicting records need local review and must not grant remote motor permission.
- `tests/modbus_settings_native.cpp`: original header compiled with actual **g++/clang++**, F_CPU=8/16MHz source macro, UBSan; **126/126 checks × 4 PASS**; interrupted writes and invalid settings included. F_CPU flags in host compilation do **NOT** prove AVR binary correctness.
- `manuals/ENGINEERING_MANUAL.md`: interface pin assignment, serial framing, EEPROM layout, proposed register map, safety locks and acceptance gates.
- `manuals/USER_MANUAL.md`: planned operator UI and default settings.

**Unfinished:** v1.06 integrated firmware, visible I²C 16×2 LCD, CRC/RTU UART dispatch, PLC tests, real AVR 8/16MHz builds and hardware tests: **NOT_RUN**. Do not flash these pure modules as firmware. v1.05 is unaffected; industrial RELEASE_BLOCKED.

## User Manual, Engineering Manual, and v1.05 archival decision (2026-11-02)

Existing Turkish guide `docs/Peristaltik_Pompa_Kullanici_Kilavuzu_v1.04.pdf` is preserved; a **Modbus-only addendum** is drafted at `docs/Peristaltik_Pompa_Modbus_Eki_v1.06.md`. An extra v1.06 User Manual is not needed. Engineering Manual will be written **last**, after integrated firmware/register/PLC/hardware acceptance; earlier preliminary manual previews were removed to avoid misrepresenting the draft as a finished product.

See `V106_TASKS.md`. User also requests **archiving the entire v1.05 folder and all related files** when v1.06 safely takes over. Do **not** move the current v1.05 source path while the P0-04 automation depends on it, or before explicit policy/automation transition and necessary acceptance gates. v1.04 archive remains unchanged.

## v1.06 I²C LCD + ClickEncoder interface adapter (scoped, not flashable)

`modbus_i2c_clickencoder_ui.h` calls the actual library API names `hd44780_I2Cexp::begin(16,2)`, `setCursor()`, `print()` and handles `ClickEncoder::Clicked`, `Held`, `Released`, `DoubleClicked`. It is **not linked into the active pump sketch**. It neither sets RS485/UART nor starts or stops the motor. The sketch must route already-read encoder events and filtered detents **exactly once** to this submenu, keep its Timer1 1-ms service, and preempt submenu rendering if ownership/safety is lost.

Native tests (`tests/modbus_i2c_clickencoder_ui_native.cpp`) compile against explicitly labeled **API signature stubs**, not the real Arduino libraries or actual I²C hardware. gcc/clang host × F_CPU 8/16 MHz macro: **533/533 PASS each**; scoped safety negative mutations: **4/4 rejected**. Existing menu bridge 622/622 ×4 and settings 126/126 ×4 re-PASS. Full AVR (both targets), original motor sketch, real PCF8574 device, timing, and physical safety acceptance **NOT_RUN**. The module must **not** be flashed as standalone firmware; no motor authority can be inferred. Engineering manual remains the final documentation task.
