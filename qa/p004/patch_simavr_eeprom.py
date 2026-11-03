#!/usr/bin/env python3
"""Fail-closed simAVR v1.6 EEPROM QA overlay. NOT calibrated hardware.

Apply only to a disposable copy of upstream simavr v1.6 sources; never patch
installed system libraries or PerPompa firmware. Restricted to combined
(erase + write) mode at nominal 3400 us. Validity is proven by separate tests.
"""
from __future__ import annotations
import argparse
from pathlib import Path

TAG = 'P004_QA_NOMINAL_EEPROM_DRIVER_V1'
WRITE_US = 3400


def once(source: str, old: str, new: str, label: str) -> str:
    if source.count(old) != 1:
        raise ValueError(f'{label}: upstream simavr v1.6 source signature changed; fail closed')
    return source.replace(old, new)


def patch(header: str, source: str) -> tuple[str, str]:
    if TAG in header or TAG in source:
        raise ValueError('already patched / mixed overlay; fail closed')
    if 'avr_eeprom_t' not in header or 'avr_eeprom_write(' not in source:
        raise ValueError('not simavr EEPROM source')
    header = once(header, '\tavr_int_vector_t ready;\t// EERIE vector',
        '\t/* ' + TAG + ': pending byte is committed only at end of nominal busy period. */\n'
        '\tuint16_t qa_pending_address;\n'
        '\tuint8_t qa_pending_value;\n'
        '\tuint8_t qa_pending_active;\n'
        '\tavr_int_vector_t ready;\t// EERIE vector', 'header state')
    old_ready = '''static avr_cycle_count_t avr_eei_raise(struct avr_t * avr, avr_cycle_count_t when, void * param)
{
\tavr_eeprom_t * p = (avr_eeprom_t *)param;
\tavr_raise_interrupt(p->io.avr, &p->ready);
\treturn 0;
}'''
    new_ready = '''static avr_cycle_count_t avr_eei_raise(struct avr_t * avr, avr_cycle_count_t when, void * param)
{
\tavr_eeprom_t * p = (avr_eeprom_t *)param;
\t/* P004_QA_NOMINAL_EEPROM_DRIVER_V1: completion is delayed, not request. */
\tif (!p->qa_pending_active) return 0;
\tp->eeprom[p->qa_pending_address] = p->qa_pending_value;
\tp->qa_pending_active = 0;
\tavr_regbit_clear(avr, p->eepe);
\tavr_raise_interrupt(p->io.avr, &p->ready);
\treturn 0;
}'''
    source = once(source, old_ready, new_ready, 'commit callback')
    source = once(source,
        '\tavr_core_watch_write(avr, addr, v);\n\n\tif (!eempe',
        '''\tavr_core_watch_write(avr, addr, v);
\tif (p->qa_pending_active) {
\t\t/* EEPE is hardware-owned during programming; reject overlap/reads. */
\t\tavr_regbit_set(avr, p->eepe);
\t\tavr_regbit_clear(avr, p->eempe);
\t\tavr_regbit_clear(avr, p->eere);
\t\treturn;
\t}

\tif (!eempe''', 'busy guard')
    old_write = '''\tif (eempe && avr_regbit_get(avr, p->eepe)) {\t// write operation
\t\t//\tprintf("eeprom write %04x <- %02x\\n", addr, avr->data[p->r_eedr]);
\t\tp->eeprom[ee_addr] = avr->data[p->r_eedr];
\t\t// Automatically clears that bit (?)
\t\tavr_regbit_clear(avr, p->eempe);

\t\tavr_cycle_timer_register_usec(avr, 3400, avr_eei_raise, p); // 3.4ms here
\t}'''
    new_write = '''\tif (eempe && avr_regbit_get(avr, p->eepe)) {\t// write operation
\t\t/* Only ATmega328P combined erase/write (EEPM=00) is supported in QA. */
\t\tif (avr_regbit_get(avr, p->eepm[0]) || avr_regbit_get(avr, p->eepm[1])) {
\t\t\tAVR_LOG(avr, LOG_ERROR, "P004 QA EEPROM unsupported EEPM write mode; dropped\\n");
\t\t\tavr_regbit_clear(avr, p->eempe);
\t\t\tavr_regbit_clear(avr, p->eepe);
\t\t\treturn;
\t\t}
\t\tp->qa_pending_address = ee_addr;
\t\tp->qa_pending_value = avr->data[p->r_eedr];
\t\tp->qa_pending_active = 1;
\t\tavr_regbit_clear(avr, p->eempe);
\t\t/* Nominal reference ONLY. A physical device must be calibrated separately. */
\t\tavr_cycle_timer_register_usec(avr, 3400, avr_eei_raise, p);
\t}'''
    source = once(source, old_write, new_write, 'deferred write')
    source = once(source,
        '\t// autocleared\n\tavr_regbit_clear(avr, p->eepe);\n\tavr_regbit_clear(avr, p->eere);',
        '''\t// EERE is self-clearing, EEPE stays set while nominal write is busy.
\tif (p->qa_pending_active) avr_regbit_set(avr, p->eepe);
\telse avr_regbit_clear(avr, p->eepe);
\tavr_regbit_clear(avr, p->eere);''', 'EEPE latch')
    source = once(source,
        '\tp->eeprom = malloc(p->size);',
        '\tp->qa_pending_active = 0;\n\tp->eeprom = malloc(p->size);', 'initial pending state')
    return header, source


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--upstream', type=Path, required=True, help='disposable simavr v1.6 clone root')
    ap.add_argument('--dry-run', action='store_true')
    a = ap.parse_args()
    p=a.upstream/'simavr'/'sim'
    h=p/'avr_eeprom.h';c=p/'avr_eeprom.c'
    patched_h,patched_c=patch(h.read_text(),c.read_text())
    if not a.dry_run:
        h.write_text(patched_h)
        c.write_text(patched_c)
    print('P004_QA_PATCH_APPLIED' if not a.dry_run else 'P004_QA_PATCH_SIGNATURES_MATCH')
    print('NOMINAL_WRITE_US=3400;CALIBRATED_HARDWARE=NO;PHYSICAL_NOT_RUN;RELEASE_BLOCKED')

if __name__=='__main__':main()
