# PerPompa v1.06 — hardware proposal / NOT PHYSICALLY ACCEPTED

| Device | Arduino Pro Mini | Caution |
|---|---|---|
| RS485 RO, DI | D0 RX, D1 TX | Same UART is needed for USB/serial bootloader; RS485 receiver must be isolated/disabled during upload |
| RS485 DE and active-low /RE | A1 | Tie only on matching transceiver interface, A1 LOW=receive / HIGH=transmit; external pull-down for safe boot |
| LCD SDA/SCL | A4/A5 | Verify PCF8574 I²C backpack voltage, pullups and address (often 0x27 / 0x3F) |
| Encoder | D2/D3/D4 | Preserve current behavior |
| Remote contact | D5 | Preserve current debounce/arming and HOLD/PULSE |
| Motor DIR/STEP | D6/D7 | No changes to driver/microstep timing until measured acceptance |
| RNG commanded-run | A0 | Commanded steps only; NOT evidence of fluid flow |
| Old parallel LCD | D8–D13 | Released by I²C conversion, not automatically reassigned |

Firmware targets: Pro Mini 5V/16MHz and 3.3V/8MHz. A common 5V LCD backpack and/or 5V-only RS485 module may be **unsafe** on the 3.3V controller; verify module datasheet and actual supply/logic levels first. For a field bus evaluate isolated RS485, proper topology/termination/bias, ESD/EMC and power-loss behavior. No physical wiring accepted in software tests.

Default Modbus RTU settings: Slave ID 1, 9600 baud, **8E1** (8 data, even parity, 1 stop), half-duplex 2-wire. PLC and future PC are alternative **masters**. RS485 DE LOW alone does not necessarily disconnect the RS485 RO pin from the bootloader RX line.

Suggested LCD library for future integrated sketch: hd44780_I2Cexp, PCF8574 auto-detect. Library/target toolchain acceptance pending.
