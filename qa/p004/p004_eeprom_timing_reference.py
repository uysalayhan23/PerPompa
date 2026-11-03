#!/usr/bin/env python3
"""P0-04 independent EEPROM timing *reference model*, NOT a hardware acceptance gate.

ATmega328P combined erase/write nominal time is 3.4 ms; EEPE remains busy
until completion. CPU interrupts can execute during EEPROM programming, but
interrupt service durations, real program call graph, physical tolerances,
reset/brownout and ISR nesting are OUT OF SCOPE. Nothing here drives hardware.
"""
from dataclasses import dataclass, field
from fractions import Fraction

NOMINAL_WRITE_US = 3400
EEMPE_WINDOW_CYCLES = 4

@dataclass
class EepromTimingModel:
    cpu_hz: int
    memory: bytearray = field(default_factory=lambda: bytearray([0xff] * 1024))
    cycles: int = 0
    busy_until: int = 0
    pending: tuple[int, int] | None = None
    armed_at: int | None = None
    events: list[tuple[str, int]] = field(default_factory=list)
    motor_step_high: bool = False  # Test fixture invariant; not firmware simulation.

    def __post_init__(self):
        if self.cpu_hz not in (8_000_000, 16_000_000):
            raise ValueError('Unsupported QA clock')

    @property
    def busy(self) -> bool:
        return self.pending is not None

    @property
    def nominal_write_cycles(self) -> int:
        return self.cpu_hz * NOMINAL_WRITE_US // 1_000_000

    @property
    def time_us(self) -> Fraction:
        return Fraction(self.cycles * 1_000_000, self.cpu_hz)

    def arm(self):
        if self.busy:
            return False
        self.armed_at = self.cycles
        return True

    def request_write(self, address: int, data: int) -> bool:
        if not (0 <= address < len(self.memory) and 0 <= data <= 255):
            raise ValueError('EEPROM address/data out of range')
        if self.busy or self.armed_at is None:
            return False
        if not (0 <= self.cycles - self.armed_at < EEMPE_WINDOW_CYCLES):
            self.armed_at = None
            return False
        self.armed_at = None
        self.pending = (address, data)
        self.busy_until = self.cycles + self.nominal_write_cycles
        self.events.append(('EEPROM_WRITE_START', self.cycles))
        return True

    def advance(self, cycles: int):
        if cycles < 0:
            raise ValueError('Clock cannot run backwards')
        self.cycles += cycles
        if self.pending is not None and self.cycles >= self.busy_until:
            address, data = self.pending
            self.memory[address] = data
            self.pending = None
            self.events.append(('EEPROM_COMMIT', self.busy_until))

    def interrupt(self, irq: str):
        if irq not in ('UART_RX_ISR', 'TIMER1_ISR'):
            raise ValueError('Unknown IRQ')
        # Interrupts are independent CPU events; no claim about firmware ISR behavior.
        self.events.append((irq, self.cycles))

    def reset_during_write(self):
        # Hardware byte result on power loss is unspecified here. Fail closed.
        if self.busy:
            self.events.append(('INTERRUPTED_WRITE_UNVERIFIED', self.cycles))
            self.pending = None
            self.armed_at = None
            return 'RECOVERY_REVIEW'
        return 'NO_PENDING_WRITE'

    def overlap_evidence(self) -> dict:
        starts = [t for name, t in self.events if name == 'EEPROM_WRITE_START']
        commits = [t for name, t in self.events if name == 'EEPROM_COMMIT']
        if len(starts) != 1 or len(commits) != 1:
            return dict(model_overlap='NOT_PROVEN', reason='Incomplete or ambiguous write')
        begin, end = starts[0], commits[0]
        uart = [t for name, t in self.events if name == 'UART_RX_ISR' and begin <= t < end]
        timer = [t for name, t in self.events if name == 'TIMER1_ISR' and begin <= t < end]
        return dict(model_overlap='MODEL_EVENT_OVERLAP' if uart and timer else 'NOT_PROVEN',
                    uart_in_busy=len(uart), timer_in_busy=len(timer),
                    duration_cycles=end-begin, nominal_write_us=NOMINAL_WRITE_US,
                    physical_acceptance='REVIEW', full_stack_acceptance='REVIEW')
