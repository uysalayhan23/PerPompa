# PerPompa v1.06 — proposed register map v0.1 (NOT IMPLEMENTED)

All addresses are **zero-based PDU addresses**: a PLC may display holding 0 as 40001, input 0 as 30001. 32-bit numbers use **MSW first**. Canonical volume **µL**, flow **µL/min**. Functions planned: 03/04 read, 06/16 guarded write. Valid CRC and frame timing mandatory; reject broadcasts for motion. Actual I/O is NOT implemented.

## Holding registers (0x03, guarded 0x06/0x10)
| Address | Meaning |
|---|---|
| 0000-0001 | uint32 staged target volume µL |
| 0002-0003 | uint32 staged speed µL/min |
| 0004 | Direction 0=CW, 1=CCW |
| 0005 | Mode 0=Dose, 1=Pump, 2=Cal (Cal requires local confirmation) |
| 0006 | Display volume unit 0=mL, 1=µL, 2=rotations |
| 0007 | Display speed unit 0=mL/min, 1=µL/min, 2=rpm |
| 0008 | D5 Remote 0=Hold, 1=Pulse |
| 0009 | Modbus Slave ID 1..247 |
| 000A | PLC watchdog ms, validated range TBD |
| 000B | Apply staged configuration, only at idle |
| 000C | Command 1=STOP, 2=START, 3=SAVE, 4=ACK_FAULT |
| 000D | Monotonic command sequence (FC16 atomic with command for START/SAVE) |
| 000E | Proposed calibration in legacy 0.001mL units, **requires local review** |
| 000F | Request owner 0=LOCAL, 1=MODBUS, gated; NEVER auto-start after boot |

## Input registers (0x04, read only)
| Address | Meaning |
|---|---|
| 0000 | Identifier 0x5050 ('PP') |
| 0001 | Firmware version 106 |
| 0002 | Status bits: 0 commanded-run, 1 dose, 2 fault, 3 Modbus owner, 4 config pending, 5 settings verified, 6 HW approved, 7 watchdog fault |
| 0003 | Last fault code (enum TBD) |
| 0004 | Mode 0 idle, 1 dose, 2 pump, 3 cal |
| 0005 | Controller 0 none, 1 local, 2 physical D5, 3 Modbus |
| 0006-0007 | Estimated µL from commanded STEP pulses; **not measured fluid flow** |
| 0008-0009 | uint32 commanded STEP counter |
| 000A | Last acknowledged command sequence |
| 000B | Communication errors saturated count |
| 000C | Last-valid-frame age ms |
| 000D | Active saved calibration, legacy 0.001 mL units |

Fail-closed: PLC START/STOP and parameter changes route through existing motor permission and validated EEPROM; **STOP always allowed**, retries with stale START sequence cannot restart; loss of Modbus supervision during Modbus-owned motion stops and inhibits automatic restart. Local/physical controls have separately enforced ownership. Parameter atomicity, apply/save ranges, watchdog timing and lifecycle are REVIEW until implemented and tested.

Future PC software uses this **versioned** map; make PLC-specific 0-based/1-based settings explicit.
