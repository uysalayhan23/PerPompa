#!/usr/bin/env bash
# P0-04 read-only QA: optimized AVR stack high-water PC attribution.
set -euo pipefail
root="$RUNNER_TEMP/p004-origin"; report="$root/report"
mkdir -p "$root/sketch" "$report"
echo 'b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2  qa/p004/read_only_candidate.tar.gz' | sha256sum -c -
tar -xzf qa/p004/read_only_candidate.tar.gz -C "$root/sketch"
ino="$root/sketch/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino"
python3 qa/p004/flash_lcd_literals_candidate.py "$ino"
echo '5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5  '"$ino" | sha256sum -c -
printf '%s\n' 'OPTIMIZED_CANDIDATE_SHA256=5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5' 'FIRMWARE_READ_ONLY=TRUE' > "$report/RESULT.txt"
getaddr() { awk -v wanted="$2" '$3==wanted {v=$1;sub(/^0080/,"",v);print v;exit}' "$1"; }
for entry in '16MHz arduino:avr:pro:cpu=16MHzatmega328 16000000' '8MHz arduino:avr:pro:cpu=8MHzatmega328 8000000'; do
  read -r clk fqbn hz <<< "$entry"
  sketch="$root/sketch/Peristaltic_Pump_Software_v1.06"
  build="$root/build-$clk"
  arduino-cli compile --fqbn "$fqbn" --build-path "$build" "$sketch" > "$report/compile-$clk.log" 2>&1
  grep -E 'Sketch uses|Global variables use' "$report/compile-$clk.log" | tee -a "$report/RESULT.txt"
  elf=$(find "$build" -name '*.elf' -type f -print -quit)
  test -f "$elf"
  avr-nm -n -C "$elf" > "$report/symbols-$clk.txt"
  avr-size -A "$elf" > "$report/sections-$clk.txt"
  avr-objdump -C -d "$elf" > "$report/disasm-$clk.txt"
  brk=$(getaddr "$report/symbols-$clk.txt" '__brkval')
  bss=$(getaddr "$report/symbols-$clk.txt" '__bss_end')
  lcd=$(getaddr "$report/symbols-$clk.txt" 'lcd_init_ok')
  menu=$(getaddr "$report/symbols-$clk.txt" 'menu_number_1')
  hw=$(getaddr "$report/symbols-$clk.txt" 'hwUiActive')
  test -n "$brk" && test -n "$bss" || { echo 'MISSING_SRAM_SYMBOLS' >&2; exit 4; }
  timeout 60s "$root/stack_origin_probe" "$elf" "$hz" "$brk" "$bss" "${lcd:-0}" "${menu:-0}" "${hw:-0}" > "$report/simavr-$clk.txt"
  grep -E 'SIMAVR_(STACK_DEPTH_OBSERVED|STACK_HEAP_GAP_OBSERVED|I2C_WRITES|LCD_INIT_OBSERVED|INPUT_STIMULUS_STAGES)|TRACE_TOTAL|TRACE_KEPT' "$report/simavr-$clk.txt" | tee -a "$report/RESULT.txt"
done
python3 qa/p004/trace_pc_origin_review.py "$report" | tee "$report/PC_ORIGIN_RESULT.txt"
python3 qa/p004/check_atomic_sp_review.py "$report" | tee "$report/ATOMIC_SP_RESULT.txt"
printf '%s\n' 'STACK_ORIGIN_ATTRIBUTION=BOUNDED_TRACE_ONLY' 'TRUE_ISR_NESTING_AND_ALL_PATHS=REVIEW' 'PHYSICAL_NOT_RUN;RELEASE_BLOCKED;NO_UPLOAD' | tee -a "$report/RESULT.txt"
