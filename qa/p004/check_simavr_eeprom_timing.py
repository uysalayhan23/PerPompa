#!/usr/bin/env python3
"""P0-04 QA-only fail-closed check of simAVR internal EEPROM timing model.

Never treats source-inspection alone as a calibrated EEPROM timing PASS.
No firmware modifications, physical flashing, network access or runner triggers.
"""
import argparse
import json
from pathlib import Path
import re


def inspect(source: str) -> dict:
    def region(start: str, end: str) -> str:
        a = source.find(start)
        if a < 0:
            return ""
        b = source.find(end, a + len(start))
        return source[a:b] if b >= 0 else source[a:]

    write = region("avr_eeprom_write(", "avr_eeprom_ioctl(")
    ready = region("avr_eei_raise(", "avr_eeprom_write(")
    # The current 1.6 model writes the backing array immediately in the
    # EECR write callback and clears EEPE, not when the 3.4 ms timer fires.
    immediate_data = bool(re.search(
        r"p\s*->\s*eeprom\s*\[\s*ee_addr\s*\]\s*=\s*avr\s*->\s*data\s*\[\s*p\s*->\s*r_eedr\s*\]", write))
    early_busy_clear = bool(re.search(
        r"avr_regbit_clear\s*\(\s*avr\s*,\s*p\s*->\s*eepe\s*\)", write))
    irq_timer_3400us = bool(re.search(
        r"avr_cycle_timer_register_usec\s*\(\s*avr\s*,\s*3400\s*,\s*avr_eei_raise\s*,\s*p\s*\)", write))
    ready_callback_clears_busy = bool(re.search(
        r"avr_regbit_clear\s*\(\s*(?:avr|p\s*->\s*io\s*\.\s*avr)\s*,\s*p\s*->\s*eepe\s*\)", ready))
    known_fast = (immediate_data and early_busy_clear and irq_timer_3400us
                  and not ready_callback_clears_busy)
    return {
        "status": "UNREALISTIC_INSTANT_EEPROM_WRITE_BLOCK" if known_fast
                  else "TIMING_MODEL_UNVERIFIED_HOLD",
        "data_written_in_EECR_callback": immediate_data,
        "EEPE_cleared_in_EECR_callback": early_busy_clear,
        "ready_IRQ_timer_3400us": irq_timer_3400us,
        "EEPE_cleared_by_ready_callback": ready_callback_clears_busy,
        "combined_UART_Timer_EEPROM_physical_timing_acceptance": "BLOCKED",
        "physical_test": "NOT_RUN",
        "release": "RELEASE_BLOCKED",
    }


def self_test() -> None:
    fixture = '''static avr_cycle_count_t avr_eei_raise(avr_t *avr) {avr_raise_interrupt(avr, &p->ready);}
static void avr_eeprom_write(avr_t *avr) {
 p->eeprom[ee_addr] = avr->data[p->r_eedr];
 avr_cycle_timer_register_usec(avr, 3400, avr_eei_raise, p);
 avr_regbit_clear(avr, p->eepe);
}
static int avr_eeprom_ioctl(avr_t *avr) { return 0; }
'''
    cases = (
        ("instant model rejected", fixture, "UNREALISTIC_INSTANT_EEPROM_WRITE_BLOCK"),
        ("deferred data unknown", fixture.replace("p->eeprom[ee_addr] = avr->data[p->r_eedr];", "/* pending data */"), "TIMING_MODEL_UNVERIFIED_HOLD"),
        ("deferred busy unknown", fixture.replace("avr_regbit_clear(avr, p->eepe);", "/* pending EEPE */"), "TIMING_MODEL_UNVERIFIED_HOLD"),
        ("missing timer unknown", fixture.replace("avr_cycle_timer_register_usec(avr, 3400, avr_eei_raise, p);", "/* missing timer */"), "TIMING_MODEL_UNVERIFIED_HOLD"),
        ("missing callback unknown", fixture.replace("avr_eeprom_write(", "unrecognized_write("), "TIMING_MODEL_UNVERIFIED_HOLD"),
    )
    for label, source, status in cases:
        result = inspect(source)
        if result["status"] != status or result["release"] != "RELEASE_BLOCKED":
            raise AssertionError("Fail-closed check failed: " + label)
    print("P004_EEPROM_TIMING_GATE_SELF_TEST=5/5_PASS (synthetic reference/mutations)")


def main() -> int:
    ap = argparse.ArgumentParser()
    mode = ap.add_mutually_exclusive_group(required=True)
    mode.add_argument("--self-test", action="store_true")
    mode.add_argument("--source", type=Path, help="local simavr/sim/avr_eeprom.c source file")
    args = ap.parse_args()
    if args.self_test:
        self_test()
        return 0
    result = inspect(args.source.read_text(encoding="utf-8"))
    result["examined_file"] = str(args.source)
    print(json.dumps(result, sort_keys=True, indent=2))
    # A model must be calibrated independently; never emit source-only PASS.
    return 2 if result["status"] == "UNREALISTIC_INSTANT_EEPROM_WRITE_BLOCK" else 3


if __name__ == "__main__":
    raise SystemExit(main())
