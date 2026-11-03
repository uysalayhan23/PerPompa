/* P0-04 simulator-only SRAM probe. Never programs or modifies a physical board. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "simavr/sim_avr.h"
#include "simavr/sim_elf.h"
#include "simavr/avr_twi.h"
#include "simavr/avr_ioport.h"

typedef struct {
  avr_irq_t *input;
  uint8_t latch;
  uint64_t rx;
  uint64_t tx;
} pcf8574_t;

static void pcf_rx(struct avr_irq_t *irq, uint32_t value, void *param) {
  (void)irq;
  pcf8574_t *p = (pcf8574_t *)param;
  avr_twi_msg_irq_t m;
  m.u.v = value;
  const uint8_t op = m.u.twi.msg;
  if ((m.u.twi.addr & 0xfeu) != 0x4eu) return; /* PCF8574 at 0x27 */
  if (op & TWI_COND_START) {
    avr_raise_irq(p->input, avr_twi_irq_msg(TWI_COND_ACK, m.u.twi.addr, 1));
  } else if (op & TWI_COND_WRITE) {
    p->latch = m.u.twi.data;
    p->rx++;
    avr_raise_irq(p->input, avr_twi_irq_msg(TWI_COND_ACK, m.u.twi.addr, 1));
  } else if (op & TWI_COND_READ) {
    p->tx++;
    avr_raise_irq(p->input, avr_twi_irq_msg(TWI_COND_READ | TWI_COND_ACK, m.u.twi.addr, p->latch));
  }
}
static uint16_t read16(avr_t *a, uint16_t addr) {
  return (uint16_t)a->data[addr] | ((uint16_t)a->data[addr+1] << 8);
}
int main(int argc, char **argv) {
  if (argc != 8) {
    fprintf(stderr,"Usage: probe ELF FREQ HBRK BSS LCD MENU HWUI\n");
    return 2;
  }
  elf_firmware_t firmware = {0};
  if (elf_read_firmware(argv[1], &firmware) != 0) return 3;
  uint32_t hz = (uint32_t)strtoul(argv[2],0,10);
  const uint16_t brkaddr = (uint16_t)strtoul(argv[3],0,16);
  const uint16_t bssend = (uint16_t)strtoul(argv[4],0,16);
  const uint16_t lcdaddr = (uint16_t)strtoul(argv[5],0,16);
  const uint16_t menuaddr = (uint16_t)strtoul(argv[6],0,16);
  const uint16_t hwaddr = (uint16_t)strtoul(argv[7],0,16);
  avr_t *avr = avr_make_mcu_by_name("atmega328p");
  if (!avr) return 4;
  avr_init(avr);
  avr->frequency = hz;
  avr_load_firmware(avr, &firmware);
  avr_irq_t *button=avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2);
  avr_irq_t *remote=avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 5);
  if (button) avr_raise_irq(button, 1);
  if (remote) avr_raise_irq(remote, 1);
  pcf8574_t pcf={0};
  pcf.latch = 0xff;
  pcf.input=avr_io_getirq(avr, AVR_IOCTL_TWI_GETIRQ(0), TWI_IRQ_INPUT);
  avr_irq_t *output=avr_io_getirq(avr, AVR_IOCTL_TWI_GETIRQ(0), TWI_IRQ_OUTPUT);
  if (pcf.input && output)
    avr_irq_register_notify(output, pcf_rx, &pcf);
  uint16_t low=avr->ramend, highheap=bssend;
  uint16_t committed_low=avr->ramend;
  uint32_t committed_pc=0;
  uint64_t committed_cycle=0;
  uint8_t committed_phase=0;
  unsigned int sph_writes=0, spl_writes=0, transient_samples=0;
  int sp_partial=0, sp_partial_age=0, sp_unpaired=0;
  uint64_t steps=0;
  int state=0, lcd_seen=0, heap_seen=0, hw_seen=0, stimulus=0;
  /* High-water diagnostics: capture AVR PC precisely when SP makes a new low. */
  struct trace_point {uint16_t sp;uint32_t pc;uint64_t cycle;uint8_t phase;};
  struct trace_point trace[512]={0};
  uint32_t trace_total=0;unsigned trace_kept=0;
  uint16_t phase_min[7];
  for(unsigned i=0;i<7;i++)phase_min[i]=avr->ramend;
  const uint64_t limit=70000000;
  for (steps=0; steps<limit; steps++) {
    state=avr_run(avr);
    uint16_t sp=read16(avr, 0x5d); /* SPL at 0x5d, SPH at 0x5e */
    /* Sample the executed 16-bit OUT opcode by inspecting flash at next PC-2.
       OUT SPH (I/O 0x3e) starts an atomic two-byte stack-pointer change.
       OUT SPL (I/O 0x3d) completes it. Do not count the intervening partial
       SPH+old-SPL value as allocated stack depth. */
    if (avr->pc>=2 && avr->pc<32768) {
      uint32_t prev_pc=avr->pc-2;
      uint16_t opcode=(uint16_t)avr->flash[prev_pc] | ((uint16_t)avr->flash[prev_pc+1]<<8);
      if ((opcode & 0xf800U) == 0xb800U) {
        uint8_t ioaddr=(uint8_t)(((opcode>>5)&0x30U)|(opcode&0x0fU));
        if (ioaddr==0x3e) {sp_partial=1;sp_partial_age=0;sph_writes++;}
        else if (ioaddr==0x3d) {sp_partial=0;sp_partial_age=0;spl_writes++;}
      }
    }
    if (sp_partial) {
      transient_samples++;
      if (++sp_partial_age>8) sp_unpaired++;
    } else if (sp>=0x100 && sp<=avr->ramend && sp<committed_low) {
      committed_low=sp;committed_pc=avr->pc;
      committed_cycle=avr->cycle;committed_phase=(uint8_t)stimulus;
    }
    if (sp >= 0x100 && sp <= avr->ramend) {
      if (sp < phase_min[stimulus]) phase_min[stimulus]=sp;
      if (sp < low) {
        low=sp;
        trace_total++;
        if(trace_kept < sizeof(trace)/sizeof(trace[0])) {
          trace[trace_kept].sp=sp;
          trace[trace_kept].pc=avr->pc;
          trace[trace_kept].cycle=avr->cycle;
          trace[trace_kept].phase=(uint8_t)stimulus;
          trace_kept++;
        }
      }
    }
    if (brkaddr >= 0x100 && brkaddr < avr->ramend) {
      uint16_t brk=read16(avr, brkaddr);
      if (brk >= bssend && brk <= avr->ramend) {
        heap_seen=1;
        if (brk > highheap) highheap=brk;
      }
    }
    if (lcdaddr >= 0x100 && lcdaddr < avr->ramend && avr->data[lcdaddr])
      lcd_seen=1;
    if (hwaddr >= 0x100 && hwaddr < avr->ramend && avr->data[hwaddr]) hw_seen=1;
    if (lcd_seen && button && remote && menuaddr >= 0x100 && menuaddr < avr->ramend && avr->cycle >= 11000000 && stimulus==0) {
      avr->data[menuaddr]=11; avr->data[menuaddr+1]=0;
      avr_raise_irq(button,0); stimulus=1;
    }
    if (stimulus==1 && avr->cycle >= 30000000) {avr_raise_irq(button,1); stimulus=2;}
    if (stimulus==2 && avr->cycle >= 35000000) {avr_raise_irq(remote,0); stimulus=3;}
    if (stimulus==3 && avr->cycle >= 39000000) {avr_raise_irq(remote,1); stimulus=4;}
    if (stimulus==4 && avr->cycle >= 45000000) {avr_raise_irq(button,0); stimulus=5;}
    if (stimulus==5 && avr->cycle >= 66000000) {avr_raise_irq(button,1); stimulus=6;}
    if (state==cpu_Crashed || state==cpu_Done) break;
  }
  int remaining=(int)low - (int)highheap;
  printf("SP_RAW_LOW=0x%04x\nSP_COMMITTED_LOW=0x%04x\n",
         low,committed_low);
  printf("SP_COMMITTED_PC=0x%08" PRIx32 "\nSP_COMMITTED_CYCLE=%" PRIu64 "\nSP_COMMITTED_PHASE=%u\n",
         committed_pc,committed_cycle,committed_phase);
  printf("SP_SPH_WRITES=%u\nSP_SPL_WRITES=%u\nSP_PARTIAL_SAMPLES=%u\nSP_UNPAIRED=%d\n",
         sph_writes,spl_writes,transient_samples,sp_unpaired);
  printf("SP_COMMITTED_STACK_DEPTH_OBSERVED=%d\nSP_COMMITTED_HEAP_GAP_OBSERVED=%d\n",
         (int)avr->ramend-(int)committed_low,(int)committed_low-(int)highheap);
  printf("TRACE_TOTAL=%" PRIu32 "\nTRACE_KEPT=%u\n",trace_total,trace_kept);
  for(unsigned i=0;i<trace_kept;i++) {
    printf("TRACE_LOW=%u,0x%04x,0x%08" PRIx32 ",%" PRIu64 ",%u\n",
           i,trace[i].sp,trace[i].pc,trace[i].cycle,trace[i].phase);
  }
  for(unsigned i=0;i<7;i++)
    printf("TRACE_PHASE=%u,0x%04x\n",i,phase_min[i]);
  printf("SIMAVR_MODEL=atmega328p\nSIMAVR_FREQ=%" PRIu32 "\n",hz);
  printf("SIMAVR_STEPS=%" PRIu64 "\nSIMAVR_CPU_CYCLES=%" PRIu64 "\n",steps,(uint64_t)avr->cycle);
  printf("SIMAVR_SP_MIN=0x%04x\nSIMAVR_STACK_DEPTH_OBSERVED=%d\n",low,(int)avr->ramend-(int)low);
  printf("SIMAVR_BSS_END=0x%04x\nSIMAVR_HEAP_BREAK_HWM=0x%04x\nSIMAVR_HEAP_BRK_OBSERVED=%d\n",bssend,highheap,heap_seen);
  printf("SIMAVR_STACK_HEAP_GAP_OBSERVED=%d\n",remaining);
  printf("SIMAVR_LCD_INIT_OBSERVED=%d\nSIMAVR_I2C_WRITES=%" PRIu64 "\nSIMAVR_I2C_READS=%" PRIu64 "\n",lcd_seen,pcf.rx,pcf.tx);
  printf("SIMAVR_CPU_STATE=%d\nSIMAVR_INPUT_STIMULUS_STAGES=%d\nSIMAVR_HW_UI_OBSERVED=%d\n",state,stimulus,hw_seen);
  printf("SIMAVR_SCOPE=BOOT_LCD_ENCODER_REMOTE_SCRIPT_NOT_EXHAUSTIVE_OR_PHYSICAL_PEAK\n");
  avr_terminate(avr);
  return (state==cpu_Crashed || remaining < 0) ? 6 : 0;
}
