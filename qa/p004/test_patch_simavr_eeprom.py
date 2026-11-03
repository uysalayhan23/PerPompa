#!/usr/bin/env python3
"""QA-only tests for simAVR v1.6 patched EEPROM driver, no physical device."""
from pathlib import Path
import os
import unittest
from patch_simavr_eeprom import patch, TAG

ROOT = Path(os.environ.get('P004_UPSTREAM_DIR', Path(__file__).parent / 'fixtures'))
SRC = ROOT / 'simavr' / 'sim'


class TestPatch(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.header = (SRC / 'avr_eeprom.h').read_text()
        cls.source = (SRC / 'avr_eeprom.c').read_text()

    def test_01_unpatched_rejected_by_busy_criteria(self):
        self.assertNotIn(TAG, self.source)
        self.assertIn('p->eeprom[ee_addr] = avr->data[p->r_eedr]', self.source)
        self.assertIn('avr_regbit_clear(avr, p->eepe);', self.source)
        self.assertNotIn('qa_pending_active', self.source)

    def test_02_qa_patch_is_single_and_deferred(self):
        header, source = patch(self.header, self.source)
        self.assertIn(TAG, header)
        self.assertIn(TAG, source)
        self.assertIn('p->eeprom[p->qa_pending_address] = p->qa_pending_value;', source)
        self.assertNotIn('p->eeprom[ee_addr] = avr->data[p->r_eedr];', source)
        self.assertIn('avr_cycle_timer_register_usec(avr, 3400, avr_eei_raise, p)', source)
        self.assertIn('if (p->qa_pending_active) avr_regbit_set(avr, p->eepe);', source)
        self.assertIn('p->qa_pending_active = 0;', source)
        self.assertIn('avr_regbit_get(avr, p->eepm[1])', source)

    def test_03_repatch_fails_closed(self):
        header, source = patch(self.header, self.source)
        with self.assertRaises(ValueError):
            patch(header, source)

    def test_04_mutated_upstream_callback_rejected(self):
        source = self.source.replace('avr_raise_interrupt(p->io.avr, &p->ready);', '/* OMIT IRQ */', 1)
        with self.assertRaisesRegex(ValueError, 'commit callback'):
            patch(self.header, source)

    def test_05_mutated_immediate_write_rejected(self):
        source = self.source.replace('p->eeprom[ee_addr] = avr->data[p->r_eedr];', '/* different EEPROM writer */', 1)
        with self.assertRaisesRegex(ValueError, 'deferred write'):
            patch(self.header, source)

    def test_06_missing_or_duplicate_header_rejected(self):
        with self.assertRaisesRegex(ValueError, 'header state'):
            patch(self.header.replace('avr_int_vector_t ready;', 'avr_int_vector_t replaced;'), self.source)
        with self.assertRaisesRegex(ValueError, 'header state'):
            patch(self.header.replace('\tavr_int_vector_t ready;\t// EERIE vector', '\tavr_int_vector_t ready;\t// EERIE vector\n\tavr_int_vector_t ready;\t// EERIE vector'), self.source)

    def test_07_mode_and_busy_guards_must_exist(self):
        _,source=patch(self.header,self.source)
        marker=source.index('if (p->qa_pending_active) {')
        self.assertLess(marker,source.index('if (!eempe && avr_regbit_get(avr, p->eempe))'))
        self.assertLess(marker,source.index('if (eempe && avr_regbit_get(avr, p->eepe))'))
        self.assertIn('unsupported EEPM write mode',source)


if __name__=='__main__':
    unittest.main(verbosity=2)
