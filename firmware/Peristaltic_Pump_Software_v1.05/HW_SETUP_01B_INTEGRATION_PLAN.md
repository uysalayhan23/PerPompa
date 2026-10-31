# HW-SETUP-01B — test-first integration handoff (2026-10-31)

**Repository `main` basis:** `0e2ff55983912bda580722b98f8e41f53d245bfc`.
**Locked accepted P0-01 `.ino` Git blob:** `6617cc38bde9bddf1c32cd0506915c0054556127`.
**Scope:** This is an independent, compiled C++11 adapter and an integration plan; **active `.ino` is not changed**. No motor can be rendered safer by an unreferenced header. Keep `RELEASE_BLOCKED` and `HW_BENCH/ HW_PUMP=NOT_RUN`.

## Safe adapter behavior

- `hw-setup-runtime.h` uses the existing `hw-setup-profile.h` without changing its accepted algorithms. EEPROM legacy addresses **0–21** remain untouched. Versioned dual-slot profile bytes remain at **32–43** and **48–59**. Independent arming byte is **64**, initially unarmed (erased `0xFF`), and is verified before authorizing a pulse.
- A4988 is an **unconfirmed suggestion**; Board remains unknown until explicitly selected. DRV8825 remains a selectable alternative. CPU profile must match compile-time `F_CPU`; 1/4 microstep remains fixed at 800 commands per motor revolution.
- `SafeSetup::enter()` requires motor, Remote and USB to be idle, inhibits new commands and invalidates the durable arming byte. `confirmAndSave()` requires an explicit confirmation action, matching `F_CPU`, and idle controls. It invalidates the destination slot, writes bytes 0–10, writes the commit byte last, reloads/compares the newest record, and **only then** re-arms the separate byte. Cancel leaves the motor inhibited until re-confirmation. EEPROM failed writes result in rejection.
- `permit()` handles *nine* start-path enumerations covering Local Dose/Pump/Cal, Remote Dose/Pump/Cal and USB Dose/Pump/Cal. This is a reusable guard only: the real START and STEP code does not call it yet. `lcdRows()` renders fixed 16-character rows for board choice, driver choice, explicit physical confirmation, mismatch and status. It is **not** connected to the physical LiquidCrystal controller.

## Exact current `.ino` integration sites requiring separate acceptance

1. `setup()`: include `hw-setup-runtime.h`, initialize the state with EEPROM and `F_CPU`, call `boot()` before any motor command, and display `SETUP REQUIRED` with the actual LCD. Also prohibit automatically starting after reset. Existing legacy EEPROM `for i<=menu_items_limit` must be limited to 0–10 if new menu index 11 is appended. EEPROM 0–21 and the existing eleven menu values must retain their meaning.
2. UI: append a **new** `HW Setup` action at menu index 11 while retaining existing 0–10 menu indices. Encoder gestures for Board/Driver/Confirm/Cancel must be explicit, and must not cause start, `STEP` or RNG pulses. Do not assume the selected Board changes physical voltage or clock, or the driver selection changes physical jumpers/current limit.
3. Central pulses: gate **before the actual step transition** in `dose()`, `pump()` and local calibration; also gate command entry in local `case 0`, Remote `remote_start()` and USB `case 9` for p/d/c. An unauthorized state must set STEP LOW, RNG idle, clear USB run and leave the relevant action; ensure legitimate STOP remains available. A new guard must not delay STOP or silently change the accepted 30-rotation calibration.
4. EEPROM `save` action at index 8 and USB `w` remain legacy only; setup profile must be committed exclusively through its explicit Save+Confirm page. Reboot after torn/non-current/same-generation-conflicting record must remain inhibited.
5. Verify 16×2 LCD widths, button debounce/guard, Remote Hold/Pulse, USB p/d/c/w/x, local Dose/Pump/Cal, direction, RNG and error/STOP timing. Never infer real driver identity, microstep wiring or actual oscillator from menu values.

## QA evidence and unresolved gates

The independent adapter's C++11 test `tests/hw-setup-runtime-native.cpp` uses a byte-addressed fake EEPROM, calls real header methods, exercises blank/invalid/partial/corrupted records, power loss during write, sequence and CRC behavior, both drivers, `F_CPU` mismatch, all nine pulse permissions, STOP, cancel and 16×2 labels. Test includes 16 MHz and 8 MHz variants and requires the stated counts. The test purposely weakens the arming-byte guard in a temporary mutation and proves the regression suite fails. This is not a replacement for whole `setup()/loop()` source tests.

**Status after independent implementation:** `HOST_CPP_PROFILE=125/125 ×2` retained (not rerun); `HOST_CPP_ADAPTER=486/486 ×2 PASS`; `ADAPTER_AVR_PROBE=NOT_RUN`; `ACTIVE_INO_INTEGRATION=NOT_RUN`; `INTEGRATED_FULL_SETUP_LOOP=NOT_RUN`; `INTEGRATED_AVR_8/16=NOT_RUN`; `HW_BENCH/HW_PUMP=NOT_RUN`; P0-02 stop-latency `REVIEW`; industrial release `BLOCKED`.

**Safety limitation:** A 1-byte EEPROM inhibit is not an independent electrical interlock and real power-loss while that EEPROM byte is being written has not been characterized. The software tests simulate completed-byte writes and interrupted later steps only. Brownout, power-cycle and motor cutoff require actual bench/instrument tests and a separately evaluated safety architecture before hardware or release claims.
