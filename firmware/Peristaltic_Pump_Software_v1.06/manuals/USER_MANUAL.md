# PerPompa v1.06 — User Manual (development preview)

**Revision 0.1 · 2026-11-02 · NOT YET A FLASHABLE RELEASE**

PerPompa v1.06 is being designed to support a Modbus RTU PLC connection and a 16×2 LCD with an I²C adapter. The v1.06 source in this repository is not an approved firmware build. Current v1.05 user/operating instructions still govern any existing installed device.

## Menu — Modbus Settings

The planned extra main-menu entry is **Modbus Settings**. It contains four editable items and two actions:

| Menu item | Default | Choice |
|:--|:--|:--|
| Baud Rate | **9600** | 2400 / 4800 / 9600 / 19200 / 38400 / 57600 / 115200 |
| Node Address | **1** | 1–247 |
| Parity | **EVEN** | EVEN / ODD / NONE |
| Stop Bits | **1** | 1 / 2 |
| Save + Apply | — | Review and confirm while the pump is idle |
| Back | — | Exit without saving |

**Planned controls:** turn the rotary knob to highlight a setting; click to edit; turn to change a value; click again to finish editing. Choose Save + Apply, inspect the proposed changes and hold the encoder button to confirm. Cancel or Back leaves any prior saved communications settings intact. Actual LCD/encoder integration is still pending.

The PLC, Modbus computer tool and pump must all use the same baud, parity and stop-bit format, and each pump on the bus must have its own node address. A saved communications change may interrupt an existing connection until the PLC is reconfigured. When Parity is NONE, **2 stop bits** is the standard Modbus RTU recommendation.

## Operator feedback

A future PLC interface may show whether the controller is **commanding motor steps**, current operating mode, last fault, desired volume and speed. This is not a measure of actual pumped liquid: a flow sensor would be necessary for direct flow confirmation.

## Safety and firmware update

Do not attempt to flash the unfinished v1.06 firmware. During future firmware updates the serial interface must not be contested by the RS485 module. Preserve existing STOP and power isolation procedures, and use the tested v1.05 version until commissioning and safety tests have been completed.

For electrical interfaces, EEPROM layout, protocol registers and PLC engineering details, see [ENGINEERING_MANUAL.md](ENGINEERING_MANUAL.md).
