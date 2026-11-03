/* P0-04 simulator-only SRAM probe. Never programs or modifies a physical board. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "simavr/sim_avr.h"
#include "simavr/sim_elf.h"
#include "simavr/avr_twi.h"
#include "simavr/avr_ioport.h"
#include "simavr/avr_eeprom.h"
#include "simavr/avr_uart.h"
#include <string.h>

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
  if (argc != 14) {
    fprintf(stderr,"Usage: probe ELF FREQ HBRK BSS LCD MENU HWUI REVIEWED IN_ACTION MENU_BASE UART_VECTOR TIMER_VECTOR NEGATIVE\n");
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
  const uint16_t reviewaddr=(uint16_t)strtoul(argv[8],0,16);
  const uint16_t actionaddr=(uint16_t)strtoul(argv[9],0,16);
  const uint16_t menuarray=(uint16_t)strtoul(argv[10],0,16);
  const uint16_t uart_vec=(uint16_t)strtoul(argv[11],0,16),timer_vec=(uint16_t)strtoul(argv[12],0,16);
  const int negative=atoi(argv[13]);
  if(negative<0||negative>1||!uart_vec||!timer_vec)return 2;
  avr_t *avr = avr_make_mcu_by_name("atmega328p");
  if (!avr) return 4;
  avr_init(avr);
  avr->frequency = hz;
  avr_load_firmware(avr, &firmware);
  uint8_t virgin_ee[1024];memset(virgin_ee,0xff,sizeof(virgin_ee));
  avr_eeprom_desc_t init_ee={.ee=virgin_ee,.offset=0,.size=sizeof(virgin_ee)};
  avr_ioctl(avr, AVR_IOCTL_EEPROM_SET, &init_ee);
  avr_irq_t *uart_rx=avr_io_getirq(avr, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_INPUT);
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
  uint16_t low=avr->ramend, highheap=bssend, committed=avr->ramend;
  int partial=0,partial_age=0,unpaired=0,motor_high=0,violation=0,sph=0,spl=0;
  uint64_t uart_isr=0,timer_isr=0,uart_during=0,timer_during=0;
  uint64_t marker_open_cycle=0,marker_commit_cycle=0,first_uart_during_cycle=0,first_timer_during_cycle=0;
  unsigned marker_seen=0;
  uint32_t prior_pc=0;
  uint64_t steps=0;
  int state=0, lcd_seen=0, heap_seen=0, hw_seen=0, stimulus=0;
  int ee_stage=0, ee_witness=0, ee_commit=0, uart_bytes=0;
  uint64_t next_uart_cycle=0;
  uint32_t eeprom_marker_snapshots=0;
  const uint64_t limit=70000000;
  for (steps=0; steps<limit; steps++) {
    state=avr_run(avr);
    uint16_t sp=read16(avr, 0x5d); /* SPL at 0x5d, SPH at 0x5e */
    uint32_t pc=avr->pc;
    if(pc>=2&&pc<32768){
      uint16_t ins=(uint16_t)avr->flash[pc-2]|((uint16_t)avr->flash[pc-1]<<8);
      if((ins&0xf800U)==0xb800U){
        uint8_t io=(uint8_t)(((ins>>5)&0x30U)|(ins&15U));
        if(io==0x3e){partial=1;partial_age=0;sph++;}
        else if(io==0x3d){partial=0;partial_age=0;spl++;}
      }
    }
    if(partial){if(++partial_age>8)unpaired++;}
    else if(sp>=0x100&&sp<=avr->ramend&&sp<committed)committed=sp;
    if(sp>=0x100&&sp<=avr->ramend&&sp<low)low=sp;
    const int writing=ee_stage==2&&!ee_commit;
    int unow=pc>=uart_vec&&pc<uart_vec+16U,uprev=prior_pc>=uart_vec&&prior_pc<uart_vec+16U;
    int tnow=pc>=timer_vec&&pc<timer_vec+16U,tprev=prior_pc>=timer_vec&&prior_pc<timer_vec+16U;
    if(unow&&!uprev){uart_isr++;if(writing){uart_during++;if(!first_uart_during_cycle)first_uart_during_cycle=avr->cycle;}}
    if(tnow&&!tprev){timer_isr++;if(writing){timer_during++;if(!first_timer_during_cycle)first_timer_during_cycle=avr->cycle;}}
    prior_pc=pc;
    if(stimulus>0&&(avr->data[0x2b]&0x80U))motor_high++;
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
      if (reviewaddr >= 0x100 && actionaddr >= 0x100 && menuarray >= 0x100 &&
          menuarray + 7*20 + 5 < avr->ramend) {
        // Deliberate QA-only forced menu: model local prior calibration verification.
        avr->data[menuaddr]=8; avr->data[menuaddr+1]=0;
        avr->data[reviewaddr]=1; avr->data[actionaddr]=1;
        if(negative)avr_raise_irq(remote,0); /* physically unsafe Remote input */
        avr->data[menuarray+7*20+4]=0x88; avr->data[menuarray+7*20+5]=0x13;
        avr_raise_irq(button,0); ee_stage=1;
        if(negative)next_uart_cycle=avr->cycle+32000;
      }
      stimulus=1;
    }
    /* EEPROM.update() is emulated much faster than physical programming.
       Sample while the synthetic local save is active, before the slot marker
       returns to A5. Stop dense polling after the initial QA transition. */
    if (ee_stage && !ee_commit && avr->cycle < 14000000 && (steps&63)==0) {
      eeprom_marker_snapshots++;
      avr_eeprom_desc_t ee_get={.ee=NULL,.offset=0,.size=1024};
      avr_ioctl(avr, AVR_IOCTL_EEPROM_GET, &ee_get);
      if (ee_get.ee && ee_get.size>164) {
        if (ee_get.ee[144]!=0xff) ee_witness=1;
        /* The settings save invalidates slot-A commit marker 109 FIRST, then writes
           29 payload bytes. Start UART stimulus at that marker 0, not at the
           later recovery witness 144 which can be too near final commit. */
        if (ee_stage==1 && ee_get.ee[109]==0) {
          ee_stage=2;marker_seen=1;marker_open_cycle=avr->cycle;
          next_uart_cycle=avr->cycle+1;
        }
        if (ee_get.ee[109]==0xa5 && !ee_commit) {
          ee_commit=1;marker_commit_cycle=avr->cycle;
        }
        if(negative&&(ee_get.ee[144]!=0xff||ee_get.ee[80]!=0xff||ee_get.ee[109]!=0xff))violation++;
      }
    }
    if ((ee_stage>=2 || (negative&&ee_stage==1)) && uart_rx && uart_bytes<7 && avr->cycle>=next_uart_cycle){
      static const uint8_t payload[7]={'x','@','2','|','p','|','9'};
      avr_raise_irq(uart_rx,payload[uart_bytes++]);
      next_uart_cycle=avr->cycle+32000;
    }
    if (stimulus==1 && avr->cycle >= 30000000) {avr_raise_irq(button,1); stimulus=2;}
    if (stimulus==2 && avr->cycle >= 35000000) {avr_raise_irq(remote,0); stimulus=3;}
    if (stimulus==3 && avr->cycle >= 39000000) {avr_raise_irq(remote,1); stimulus=4;}
    if (stimulus==4 && avr->cycle >= 45000000) {avr_raise_irq(button,0); stimulus=5;}
    if (stimulus==5 && avr->cycle >= 66000000) {avr_raise_irq(button,1); stimulus=6;}
    if (state==cpu_Crashed || state==cpu_Done) break;
  }
  int remaining=(int)committed - (int)highheap;
  printf("COMMITTED_STACK_SP=0x%04x\nCOMMITTED_STACK_DEPTH=%d\nCOMMITTED_STACK_HEAP_GAP=%d\n",
    committed,(int)avr->ramend-(int)committed,remaining);
  printf("SPH_WRITES=%d\nSPL_WRITES=%d\nSP_UNPAIRED=%d\n",sph,spl,unpaired);
  printf("UART_ISR_ENTRY_COUNT=%" PRIu64 "\nTIMER_ISR_ENTRY_COUNT=%" PRIu64
         "\nUART_ISR_DURING_WRITE=%" PRIu64 "\nTIMER_ISR_DURING_WRITE=%" PRIu64 "\n",
         uart_isr,timer_isr,uart_during,timer_during);
  printf("MOTOR_STEP_HIGH_SAMPLES=%d\nNEGATIVE_EEPROM_VIOLATIONS=%d\n",motor_high,violation);
  printf("SIMAVR_MODEL=atmega328p\nSIMAVR_FREQ=%" PRIu32 "\n",hz);
  printf("SIMAVR_STEPS=%" PRIu64 "\nSIMAVR_CPU_CYCLES=%" PRIu64 "\n",steps,(uint64_t)avr->cycle);
  printf("SIMAVR_SP_MIN=0x%04x\nSIMAVR_STACK_DEPTH_OBSERVED=%d\n",low,(int)avr->ramend-(int)low);
  printf("SIMAVR_BSS_END=0x%04x\nSIMAVR_HEAP_BREAK_HWM=0x%04x\nSIMAVR_HEAP_BRK_OBSERVED=%d\n",bssend,highheap,heap_seen);
  printf("SIMAVR_STACK_HEAP_GAP_OBSERVED=%d\n",remaining);
  printf("SIMAVR_LCD_INIT_OBSERVED=%d\nSIMAVR_I2C_WRITES=%" PRIu64 "\nSIMAVR_I2C_READS=%" PRIu64 "\n",lcd_seen,pcf.rx,pcf.tx);
  printf("SIMAVR_CPU_STATE=%d\nSIMAVR_INPUT_STIMULUS_STAGES=%d\nSIMAVR_HW_UI_OBSERVED=%d\n",state,stimulus,hw_seen);
  printf("SIMAVR_EEPROM_SAVE_STAGED=%d\nSIMAVR_EEPROM_WITNESS_OBSERVED=%d\nSIMAVR_EEPROM_COMMITTED=%d\nSIMAVR_UART_RX_STIMULI=%d\n",ee_stage,ee_witness,ee_commit,uart_bytes);
  printf("EE_MARKER_POLL_COUNT=%" PRIu32 "\n",eeprom_marker_snapshots);
  printf("MARKER_ZERO_SEEN=%u\nMARKER_OPEN_CYCLE=%" PRIu64 "\nMARKER_COMMIT_CYCLE=%" PRIu64
         "\nFIRST_UART_ISR_DURING_CYCLE=%" PRIu64 "\nFIRST_TIMER_ISR_DURING_CYCLE=%" PRIu64 "\n",
         marker_seen,marker_open_cycle,marker_commit_cycle,first_uart_during_cycle,first_timer_during_cycle);
  const int ok=(!negative?
    (ee_stage>=2&&ee_witness&&ee_commit&&uart_bytes==7&&uart_isr>0&&timer_isr>0&&uart_during>0&&timer_during>0):
    (ee_stage==1&&!ee_witness&&!ee_commit&&!violation&&uart_bytes==7&&uart_isr>0&&timer_isr>0));
  const int accepted=ok&&committed>highheap&&!motor_high&&!unpaired&&sph>0&&sph==spl&&state!=cpu_Crashed;
  printf("QA_SCENARIO=%s\nQA_ACCEPT=%d\n",
         negative?"REMOTE_LOW_BLOCKED":"LEGACY_FIRST_SAVE_WITH_CONCURRENT_ISR",accepted);
  printf("SIMAVR_SCOPE=SYNTHETIC_LOCAL_MENU_EVENT_WITH_REAL_TIMER_UART_ISR_NOT_PHYSICAL\n");
  avr_terminate(avr);
  return accepted?0:6;
}
