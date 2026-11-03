#!/usr/bin/env bash
set -euo pipefail
root="$RUNNER_TEMP/p004-menu"; report="$root/report"
mkdir -p "$root/base" "$report"
echo 'b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2  qa/p004/read_only_candidate.tar.gz' | sha256sum -c -
tar -xzf qa/p004/read_only_candidate.tar.gz -C "$root/base"
cp -a "$root/base" "$root/compact"
for variant in base compact; do
  ino="$root/$variant/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino"
  python3 qa/p004/flash_lcd_literals_candidate.py "$ino"
done
python3 qa/p004/compact_menu_format_candidate.py "$root/compact/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino"
echo '5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5  '"$root"'/base/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino' | sha256sum -c -
echo '87fb708ca17e33cea2d13606344605d85768b200c5d8d182e4c30c05a4d3bd71  '"$root"'/compact/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino' | sha256sum -c -
getaddr() { awk -v wanted="$2" '$3==wanted {v=$1;sub(/^0080/,"",v);print v;exit}' "$1"; }
for entry in '16MHz arduino:avr:pro:cpu=16MHzatmega328 16000000' '8MHz arduino:avr:pro:cpu=8MHzatmega328 8000000'; do
  read -r clk fqbn hz <<< "$entry"
  for variant in base compact; do
    sketch="$root/$variant/Peristaltic_Pump_Software_v1.06"
    build="$root/build-$clk-$variant"
    arduino-cli compile --fqbn "$fqbn" --build-path "$build" "$sketch" > "$report/compile-$clk-$variant.log" 2>&1
    grep -E 'Sketch uses|Global variables use' "$report/compile-$clk-$variant.log" | tee -a "$report/RESULT.txt"
    elf=$(find "$build" -name '*.elf' -type f -print -quit)
    test -f "$elf"
    avr-nm -n -C "$elf" > "$report/symbols-$clk-$variant.txt"
    avr-size -A "$elf" > "$report/sections-$clk-$variant.txt"
    brk=$(getaddr "$report/symbols-$clk-$variant.txt" '__brkval')
    bss=$(getaddr "$report/symbols-$clk-$variant.txt" '__bss_end')
    lcd=$(getaddr "$report/symbols-$clk-$variant.txt" 'lcd_init_ok')
    menu=$(getaddr "$report/symbols-$clk-$variant.txt" 'menu_number_1')
    hw=$(getaddr "$report/symbols-$clk-$variant.txt" 'hwUiActive')
    test -n "$brk" && test -n "$bss"
    if [[ -z "$lcd" ]]; then lcd=0; fi
    if [[ -z "$menu" ]]; then menu=0; fi
    if [[ -z "$hw" ]]; then hw=0; fi
    timeout 50s "$root/stack_probe" "$elf" "$hz" "$brk" "$bss" "$lcd" "$menu" "$hw" > "$report/simavr-$clk-$variant.txt" 2>&1
    echo "SIMAVR_$clk-$variant=TRACE_COMPLETE" >> "$report/RESULT.txt"
    grep -E 'SIMAVR_(SP_MIN|STACK_DEPTH_OBSERVED|HEAP_BREAK_HWM|STACK_HEAP_GAP_OBSERVED|I2C_WRITES|INPUT_STIMULUS_STAGES|LCD_INIT_OBSERVED)=' "$report/simavr-$clk-$variant.txt" | tee -a "$report/RESULT.txt"
    diag="$root/diag-$clk-$variant"
    if arduino-cli compile --fqbn "$fqbn" --build-path "$diag" --build-property 'compiler.cpp.extra_flags=-fno-lto -fstack-usage' --build-property 'compiler.c.extra_flags=-fno-lto -fstack-usage' "$sketch" > "$report/diag-$clk-$variant.log" 2>&1; then
      find "$diag" -name '*.su' -type f -exec cat {} + > "$report/frames-$clk-$variant.txt"
      grep 'format_menu_line' "$report/frames-$clk-$variant.txt" | tee -a "$report/RESULT.txt" || true
    else
      echo "DIAG_$clk-$variant=REVIEW_NO_FRAME" >> "$report/RESULT.txt"
    fi
  done
done
python3 - "$report" <<'PY'
from pathlib import Path
import re,sys,json
p=Path(sys.argv[1]); data={}
for clk in ('16MHz','8MHz'):
  for v in ('base','compact'):
    log=(p/f'compile-{clk}-{v}.log').read_text()
    sim=dict(line.split('=',1) for line in (p/f'simavr-{clk}-{v}.txt').read_text().splitlines() if line.startswith('SIMAVR_') and '=' in line)
    frames=(p/f'frames-{clk}-{v}.txt').read_text() if (p/f'frames-{clk}-{v}.txt').exists() else ''
    fval=[int(s) for s in re.findall(r'format_menu_line[^\n]*\t(\d+)\t',frames)]
    result=dict(ram=int(re.search(r'Global variables use (\d+) bytes',log).group(1)),flash=int(re.search(r'Sketch uses (\d+) bytes',log).group(1)),stack=int(sim['SIMAVR_STACK_DEPTH_OBSERVED']),gap=int(sim['SIMAVR_STACK_HEAP_GAP_OBSERVED']),lcd=int(sim['SIMAVR_LCD_INIT_OBSERVED']),i2c=int(sim['SIMAVR_I2C_WRITES']),stages=int(sim['SIMAVR_INPUT_STIMULUS_STAGES']),frame=fval)
    assert result['lcd']==1 and result['stages']==6 and result['i2c']>100
    data[clk+'-'+v]=result
  b=data[clk+'-base']; c=data[clk+'-compact']
  print(clk,"BASE",b,"COMPACT",c)
  assert c['ram']<=b['ram'], "SRAM regression"
  assert c['gap']>=b['gap'], "stack-heap gap regression"
  assert c['i2c']==b['i2c'], "LCD I2C mismatch"
(p/'COMPARISON.json').write_text(json.dumps(data,indent=2))
with (p/'RESULT.txt').open('a') as f:
  f.write('AVR_AB_4/4=PASS;LCD_BOUNDED_4/4=PASS\nSTACK_ALL_PATHS_REVIEW;PHYSICAL_NOT_RUN;RELEASE_BLOCKED;NO_FIRMWARE_UPLOAD\n')
PY
