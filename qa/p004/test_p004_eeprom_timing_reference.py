#!/usr/bin/env python3
"""Positive and negative QA reference-model tests (not firmware/hardware QA)."""
import unittest
from p004_eeprom_timing_reference import EepromTimingModel

class EepromTimeTests(unittest.TestCase):
    def test_positive_programming_and_two_interrupts_8_16(self):
        for hz in (8_000_000, 16_000_000):
            with self.subTest(hz=hz):
                m=EepromTimingModel(hz)
                old=m.memory[109]
                self.assertTrue(m.arm())
                self.assertTrue(m.request_write(109,0))
                m.advance(hz//2000)
                m.interrupt('UART_RX_ISR')
                m.advance(hz//2000)
                m.interrupt('TIMER1_ISR')
                self.assertTrue(m.busy)
                self.assertEqual(m.memory[109],old)
                m.advance(m.nominal_write_cycles)
                self.assertFalse(m.busy)
                self.assertEqual(m.memory[109],0)
                evidence=m.overlap_evidence()
                self.assertEqual(evidence['model_overlap'],'MODEL_EVENT_OVERLAP')
                self.assertEqual(evidence['duration_cycles'],54400 if hz==16_000_000 else 27200)
                self.assertEqual(evidence['physical_acceptance'],'REVIEW')
                self.assertFalse(m.motor_step_high)

    def test_negative_early_data_visibility(self):
        m=EepromTimingModel(16_000_000)
        m.arm();self.assertTrue(m.request_write(4,0x12))
        m.advance(m.nominal_write_cycles-1)
        self.assertEqual(m.memory[4],0xff)
        self.assertTrue(m.busy)
        m.advance(1)
        self.assertEqual(m.memory[4],0x12)

    def test_negative_reject_parallel_eeprom_write(self):
        m=EepromTimingModel(8_000_000)
        m.arm();self.assertTrue(m.request_write(4,0x12))
        self.assertFalse(m.arm())
        self.assertFalse(m.request_write(5,0x34))
        m.advance(m.nominal_write_cycles)
        self.assertEqual(m.memory[5],0xff)

    def test_negative_expired_eempe_window(self):
        m=EepromTimingModel(16_000_000)
        m.arm();m.advance(4)
        self.assertFalse(m.request_write(1,3))
        self.assertEqual(m.memory[1],0xff)

    def test_negative_missing_uart_cannot_pass_overlap(self):
        m=EepromTimingModel(8_000_000)
        m.arm();m.request_write(1,1)
        m.interrupt('TIMER1_ISR');m.advance(m.nominal_write_cycles)
        self.assertEqual(m.overlap_evidence()['model_overlap'],'NOT_PROVEN')

    def test_negative_uart_after_commit_does_not_count(self):
        m=EepromTimingModel(16_000_000)
        m.arm();m.request_write(1,1)
        m.interrupt('TIMER1_ISR');m.advance(m.nominal_write_cycles)
        m.interrupt('UART_RX_ISR')
        self.assertEqual(m.overlap_evidence()['uart_in_busy'],0)

    def test_negative_power_cut_never_proves_recovery(self):
        m=EepromTimingModel(16_000_000)
        m.arm();m.request_write(1,1);m.advance(1000)
        self.assertEqual(m.reset_during_write(),'RECOVERY_REVIEW')
        self.assertEqual(m.overlap_evidence()['model_overlap'],'NOT_PROVEN')
        self.assertEqual(m.memory[1],0xff) # model fixture only, not physical EEPROM guarantee

    def test_negative_unrecognized_irq_or_invalid_address(self):
        m=EepromTimingModel(8_000_000)
        with self.assertRaises(ValueError):m.interrupt('FAKE_ISR')
        m.arm()
        with self.assertRaises(ValueError):m.request_write(1024,0)

    def test_negative_unsupported_frequency(self):
        with self.assertRaises(ValueError):EepromTimingModel(1_000_000)

if __name__=='__main__':unittest.main(verbosity=2)
