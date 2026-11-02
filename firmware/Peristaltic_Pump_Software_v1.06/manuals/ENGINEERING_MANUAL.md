# PerPompa v1.06 — Engineering Manual

**Document ID:** PP-ENG-106 · **Revision:** 0.2 (development) · **Date:** 2026-11-02

**Engineering status: DEVELOPMENT / NOT FLASHABLE / NOT COMMISSIONED.** This manual documents the v1.06 communication design and independently tested menu model. The integrated v1.06 Arduino firmware, LCD wiring validation, actual RTU exchanges, motor-controlled PLC behavior, and both real AVR binaries are **NOT_RUN**. Never treat these specifications as verified equipment performance.

## 1. Platform and architecture

- Arduino Pro Mini ATmega328P, supported build targets **5 V / 16 MHz** and **3.3 V / 8 MHz**, compile frequency must match hardware. Microstep mode fixed to 1/4 in the protected v1.05 source. A4988 is the default suggested driver, DRV8825 an alternative **subject to physical hardware confirmation**.
- Slave (server) **Modbus RTU**, half-duplex 2-wire RS485, one PLC or PC master at a time. On-bus default: **address 1, baud 9600, 8 data bits, even parity, 1 stop bit (8E1)**. No unprotected motion broadcast. The pump's UART is dedicated to Modbus in normal operation; firmware upload uses the same RX/TX and requires a hardware arrangement that prevents the RS485 receiver interfering with the bootloader.
- Display 16×2 HD44780 compatible LCD with a PCF8574-based I²C backpack. The I²C address is board-dependent (often 0x27, 0x3F); module supply, SDA/SCL pull-ups and pin mapping must be verified for the selected 3.3/5 V board.
- The *physical pump is not yet tested*. D5 Remote (Hold/Pulse), A0 RNG (commanded STEP only), existing STEP/STOP/EEPROM/hardware authorizations stay guarded.

## 2. Signal and pin assignment — **proposed**, not a completed wiring acceptance

| Signal | Pro Mini | Engineering note |
|:--|:--|:--|
| UART RS485 DI (TX) | D1 / TX | Hardware UART, also used by bootloader |
| UART RS485 RO (RX) | D0 / RX | Must not fight USB-TTL programmer during upload |
| RS485 DE + /RE | A1 | Proposed one direction pin; safe low on reset (external bias needed); validate actual transceiver |
| LCD I²C SDA / SCL | A4 / A5 | Use correctly rated backpack and pull-ups |
| Encoder A / B / switch | D3 / D4 / D2 | Current v1.05 encoder assignment preserved |
| Remote input | D5 | INPUT_PULLUP, 30 ms debounce and existing arming behavior |
| STEP / DIR | D7 / D6 | Driver pins; no reassignment |
| RNG | A0 | Commanded stepping indication, **not a flow sensor** |
| Former parallel LCD | D8–D13 | Released *only after* proven I²C LCD integration |

Industrial physical interface decisions (galvanic isolation, earth/reference, ESD, EMC, terminal layout, 120 Ω end termination, fail-safe bias and cable topology) require actual module datasheets, electrical design review and bench measurement; no universal wiring/certification claim is made here. Do not connect unsupported voltage levels to Pro Mini I/O.

## 3. Modbus communication parameters

| Menu parameter | Initial value | Allowed draft values | Behavior |
|:--|:--|:--|:--|
| Baud Rate | **9600** | 2400, 4800, 9600, 19200, 38400, 57600, 115200 | Actual Arduino UART setup is NOT yet integrated. Higher rates require timing acceptance. |
| Node Address | **1** | 1–247 | A unique address per slave; address 0 broadcast is rejected for movement |
| Parity | **EVEN** | EVEN, ODD, NONE | All use 8 data bits |
| Stop Bits | **1** | 1, 2 | **8E1** default, **8N2** recommended for no parity. 8N1 may interoperate with some devices but departs from the standard recommendation. |

**Modbus Serial Line V1.02:** RTU CRC-16/Modbus low byte first; a silent interval >=3.5 character times delimits frames, >1.5 character times within a frame invalidates it. For baud above 19200, specification recommends fixed 750 µs / 1.750 ms inter-character/inter-frame timing. The current receiver prototype uses a bounded 64-byte ADU, not the protocol's full 256-byte maximum; unsupported larger requests must fail closed. Actual per-byte timing from an RX interrupt or otherwise verified arrival timestamps is still NOT_RUN. The pump's existing blocking STEP pulse generation means polling queued bytes cannot substitute for true arrival timestamps.

Official references:
- https://www.modbus.org/modbus-specifications
- https://modbus.org/docs/Modbus_over_serial_line_V1_02.pdf

## 4. 16×2 LCD menu tree and local confirmation

```
Main Menu
  ... existing Dose / Pump / Cal. / Save / Remote / HW Setup
  Modbus Settings       [proposed new root ACTION]
     Baud Rate          9600
     Node Address       1
     Parity             EVEN
     Stop Bits          1
     Save + Apply
        SAVE MODBUS CFG?
        HOLD TO CONFIRM
     Back
```

For the pure source model, encoder turning without editing navigates fields; CLICK edits the field, TURN modifies the draft, CLICK completes editing; selecting Save enters a confirmation screen; HOLD explicitly requests a save; Back exits. Save is rejected unless the motor, Remote and USB/uploader controls are all idle. The UI shows defaults even with erased EEPROM. The pure `ModbusMenu::rows()` returns two 16-character LCD rows. **Integration into the original ClickEncoder / actual I²C LCD driver is pending.**

The *proposed* UART reconfiguration sequence is: (1) verify no motor motion or queued Modbus transaction, (2) confirm physically on the local screen, (3) write and reread a CRC-protected EEPROM record, (4) tell the master to reconnect with the new parameters, (5) safely stop TX and update the UART. This sequence has **not** been implemented in v1.06 and must be tested against power interruption.

## 5. Independent EEPROM settings records (tested pure component)

Existing address ownership is kept unchanged: legacy settings 0–21, hardware profile 32–59 / inhibit 64, P0-04 settings records 80–109 and 112–141, migration witness 144–151 / 160–167.

| v1.06 block | Address range | Bytes |
|:--|:--|:--|
| Modbus slot A | 192–207 | 16 |
| Modbus slot B | 224–239 | 16 |

v1.06 record format: bytes `0–1='MB'`, `2=version 1`, `3=length 16`, `4=generation`, `5=Node Address`, `6=baud index`, `7=parity`, `8=stop bits`, `9–11=0`, `12–13=CRC16/CCITT-FALSE (LSB first) of bytes 0–11`, `14=~generation`, `15=commit marker 0xA5`, written last. Each byte is written/read back; only verified records load. A torn write must not corrupt an existing valid slot. Erased EEPROM presents factory defaults **but this is not evidence that pump/calibration settings are valid**. Invalid/conflicting records must inhibit unsafe remote motion until locally reviewed in the final integration. First-write failure recovery requires independent local confirmation. Real EEPROM and brownout acceptance NOT_RUN.

## 6. Proposed Modbus application register contract v0.1

The register numbers below are **zero-based PDU offsets**. Some PLC programming software displays holding register `0` as **40001**, and input register `0` as **30001**. Confirm addressing mode in the PLC. Multiregister `uint32` data: **most significant 16-bit register first**; integers unsigned unless specified. No register server or motor dispatch is integrated yet.

### Holding registers — FC03 read, FC06/FC16 guarded writes (planned)

| Offset (hex) | R/W meaning | Unit / encoding |
|:--|:--|:--|
| 0x0000–01 | Staged target volume | `uint32`, µL |
| 0x0002–03 | Staged speed | `uint32`, µL/min |
| 0x0004 | Direction | 0 CW, 1 CCW |
| 0x0005 | Mode | 0 Dose, 1 Pump, 2 Cal (local safety approval) |
| 0x0006 | Volume display unit | 0 mL, 1 µL, 2 rotations |
| 0x0007 | Speed display unit | 0 mL/min, 1 µL/min, 2 rpm |
| 0x0008 | D5 Remote style | 0 Hold, 1 Pulse |
| 0x0009 | Modbus Slave ID | 1–247; **local-only proposal for persistent changes**, remote writes disabled until explicitly designed |
| 0x000A | Watchdog timeout | milliseconds; valid range TBD |
| 0x000B | Apply staged process settings | valid only idle |
| 0x000C | Control command | 1 STOP, 2 START, 3 SAVE, 4 ACK fault; gated |
| 0x000D | Command sequence | requires atomic FC16 combination with START/SAVE |
| 0x000E | Calibration parameter | local review required |
| 0x000F | Control owner request | 0 local, 1 Modbus; gate checks |

### Input registers — FC04 read-only (planned)

| Offset (hex) | Information |
|:--|:--|
| 0x0000 | Product identifier 0x5050 (`PP`) |
| 0x0001 | Firmware version 106 |
| 0x0002 | Bitfield: 0 commanded-run; 1 dose; 2 fault; 3 Modbus owner; 4 config pending; 5 settings verified; 6 HW approved; 7 watchdog fault |
| 0x0003 | Last fault code (enum TBD) |
| 0x0004 | Mode: 0 idle / 1 dose / 2 pump / 3 calibration |
| 0x0005 | Owner: 0 none / 1 local / 2 physical Remote D5 / 3 Modbus |
| 0x0006–07 | Estimated delivered volume (µL; from step commands, **not flow sensor**) |
| 0x0008–09 | Commanded STEP counter |
| 0x000A | Last acknowledged command sequence |
| 0x000B | Saturating communications error counter |
| 0x000C | Age of last accepted frame (ms) |
| 0x000D | Calibration (0.001 mL units) |

**Engineering caution:** The proposal is not a PLC-ready API. Parameters like address/parity/baud are deliberately editable locally; writing them remotely can disconnect the controlling PLC. A later finalized register map must version this behavior; acceptance criteria must include invalid function, invalid address, CRC error, duplicate START sequence, race between local/remote/PLC, reset and power loss. Modbus RTU offers no cryptographic authentication or physical safety channel. Critical stop needs independent risk assessment.

## 7. Functional safety rules and remaining gates

- STOP and fail-closed motor idle have priority over START/parameter changes; no automatic restart after reset, communication loss, corrupt EEPROM or PLC reconnection.
- PLC action cannot bypass local calibration review, hardware setup/driver/clock confirmation or existing v1.05 P0-04 safety guards.
- Volume/rate calculations must remain bounded; all local/Remote/USB-to-Modbus transitions need regression and commanded-RNG checks.
- UART RX overrun, DE/RE switching, poll interval, timing, watchdog and physical fail-safe during firmware upload must be measured on both 8/16 MHz targets.
- **Acceptance matrix:** Pure Modbus RTU receiver host PASS (previous scoped tests); pure menu and settings EEPROM model host PASS (new tests); integrated Arduino sketch, I²C actual display, complete Modbus server, real PLC tests, real AVR 8/16 MHz builds and hardware emergency stop **NOT_RUN**. Industrial G01–G10 `RELEASE_BLOCKED`.
- No claim of CE/SIL/PL, industrial reliability, measured fluid flow or dose accuracy without independent physical evidence.

## 8. User-facing reference and change control

- Operating description without technical PCB / register details: `USER_MANUAL.md`.
- Detailed planned PDU register map: `../MODBUS_REGISTER_MAP.md`. Changes need explicit protocol-version documentation and tests.
- v1.04 archive and approved v1.05 firmware source **unchanged**. Root `AGENTS.md` and v1.05 revision manifest **unchanged**. Running P0-04 automation continues independently.
