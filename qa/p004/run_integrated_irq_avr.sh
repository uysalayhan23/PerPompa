#!/usr/bin/env bash
# QA-only synthetic local menu action; real AVR EEPROM/Timer/UART code.
set -euo pipefail
root="$RUNNER_TEMP/p004-irq-overlap"; report="$root/report"
mkdir -p "$root/sketch" "$report"
echo 'b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2  qa/p004/read_only_candidate.tar.gz' | sha256sum -c -
tar -xzf qa/p004/read_only_candidate.tar.gz -C "$root/sketch"
sketch="$root/sketch/Peristaltic_Pump_Software_v1.06"
python3 qa/p004/flash_lcd_literals_candidate.py "$sketch/Peristaltic_Pump_Software_v1.06.ino"
echo '5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5  '"$sketch"'/Peristaltic_Pump_Software_v1.06.ino' | sha256sum -c -
echo 'QA_SHA256=5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5' > "$report/RESULT.txt"
getaddr() { awk -v wanted="$2" '$3==wanted {v=$1;sub(/^0080/,"",v);print v;exit}' "$1"; }
for entry in '16MHz arduino:avr:pro:cpu=16MHzatmega328 16000000' '8MHz arduino:avr:pro:cpu=8MHzatmega328 8000000'; do
  read -r clk fqbn hz <<< "$entry"
  build="$root/build-$clk"
  arduino-cli compile --fqbn "$fqbn" --build-path "$build" "$sketch" > "$report/compile-$clk.log" 2>&1
  grep -E 'Sketch uses|Global variables use' "$report/compile-$clk.log" | tee -a "$report/RESULT.txt"
  elf=$(find "$build" -name '*.elf' -type f -print -quit)
  test -f "$elf"
  avr-nm -n -C "$elf" > "$report/symbols-$clk.txt"
  avr-size -A "$elf" > "$report/sections-$clk.txt"
  names=(__brkval __bss_end lcd_init_ok menu_number_1 hwUiActive migrationReviewAccepted in_action menu __vector_18 __vector_13)
  args=()
  for sym in "${names[@]}"; do
    v=$(getaddr "$report/symbols-$clk.txt" "$sym")
    if [[ -z "$v" ]]; then echo "REVIEW_MISSING_AVR_SYMBOL:$clk:$sym" >&2;exit 4;fi
    args+=("$v")
  done
  for scenario in POSITIVE REMOTE_LOW; do
    negative=0
    if [[ "$scenario" == REMOTE_LOW ]];then negative=1;fi
    timeout 55s "$root/stack_irq_probe" "$elf" "$hz" \
      "${args[0]}" "${args[1]}" "${args[2]}" "${args[3]}" "${args[4]}" \
      "${args[5]}" "${args[6]}" "${args[7]}" "${args[8]}" "${args[9]}" "$negative" \
       > "$report/avr-$clk-$scenario.txt" 2>&1 || {
         cat "$report/avr-$clk-$scenario.txt"; echo "REVIEW_SIMAVR_$clk-$scenario" >&2; exit 6;
       }
    grep -E '^(QA_SCENARIO|QA_ACCEPT|COMMITTED_STACK_DEPTH|COMMITTED_STACK_HEAP_GAP|UART_ISR_DURING_WRITE|TIMER_ISR_DURING_WRITE|SIMAVR_EEPROM_COMMITTED|NEGATIVE_EEPROM_VIOLATIONS|MOTOR_STEP_HIGH_SAMPLES)=' "$report/avr-$clk-$scenario.txt" | tee -a "$report/RESULT.txt"
  done
done
python3 - "$report" <<'PY'
from pathlib import Path
import json,sys
root=Path(sys.argv[1]);results={}
for clk in ('16MHz','8MHz'):
 for scen,expected in [('POSITIVE','LEGACY_FIRST_SAVE_WITH_CONCURRENT_ISR'),('REMOTE_LOW','REMOTE_LOW_BLOCKED')]:
  raw=(root/f'avr-{clk}-{scen}.txt').read_text()
  v=dict(line.split('=',1) for line in raw.splitlines() if '=' in line)
  assert v.get('QA_SCENARIO')==expected
  assert v.get('QA_ACCEPT')=='1',f'{clk}:{scen} acceptance failed'
  assert int(v['UART_ISR_ENTRY_COUNT'])>0 and int(v['TIMER_ISR_ENTRY_COUNT'])>0
  assert int(v['COMMITTED_STACK_HEAP_GAP'])>0
  assert v['MOTOR_STEP_HIGH_SAMPLES']=='0'
  assert v['SPH_WRITES']==v['SPL_WRITES'] and v['SP_UNPAIRED']=='0'
  assert v['SIMAVR_UART_RX_STIMULI']=='7'
  if scen=='POSITIVE':
   assert v['SIMAVR_EEPROM_WITNESS_OBSERVED']=='1' and v['SIMAVR_EEPROM_COMMITTED']=='1'
   assert int(v['UART_ISR_DURING_WRITE'])>0 and int(v['TIMER_ISR_DURING_WRITE'])>0
  else:
   assert v['SIMAVR_EEPROM_WITNESS_OBSERVED']=='0' and v['SIMAVR_EEPROM_COMMITTED']=='0'
   assert v['NEGATIVE_EEPROM_VIOLATIONS']=='0'
  results[clk+':'+scen]={k:v[k] for k in ['QA_ACCEPT','COMMITTED_STACK_DEPTH','COMMITTED_STACK_HEAP_GAP',
    'UART_ISR_ENTRY_COUNT','TIMER_ISR_ENTRY_COUNT','UART_ISR_DURING_WRITE','TIMER_ISR_DURING_WRITE',
    'SIMAVR_EEPROM_COMMITTED','MOTOR_STEP_HIGH_SAMPLES']}
  print(clk,scen,"NEW_REAL_AVR_INTERRUPT_COVERAGE_PASS",results[clk+':'+scen])
(root/'COMBINED_EEPROM_UART_TIMER_RESULTS.json').write_text(json.dumps(results,indent=2))
print('AVR_COMBINED_POSITIVE_NEGATIVE_4/4_PASS;SYNTHETIC_MENU_EVENT_NOT_PHYSICAL')
print('ALL_PATHS_STACK_ISR_REVIEW;PHYSICAL_NOT_RUN;RELEASE_BLOCKED')
PY
printf '%s\n' 'QA_ONLY_SIMAVR_COMBINED_PATH_SUCCESS' 'NO_REAL_DEVICE;PRODUCTION_UNCHANGED;PHYSICAL_NOT_RUN;RELEASE_BLOCKED' | tee -a "$report/RESULT.txt"
