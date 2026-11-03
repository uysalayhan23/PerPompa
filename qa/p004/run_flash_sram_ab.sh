#!/usr/bin/env bash
set -euo pipefail
root="$RUNNER_TEMP/p004"
report="$root/report"
mkdir -p "$root/base" "$report"
echo 'b9d04926776ce3703043c507fa804c6cc6ff6ca494f84df6b6926b37ff3d38f2  qa/p004/read_only_candidate.tar.gz' | sha256sum -c -
tar -xzf qa/p004/read_only_candidate.tar.gz -C "$root/base"
echo 'e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21  '"$root"'/base/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino' | sha256sum -c -
cp -a "$root/base" "$root/flash"
python3 qa/p004/flash_lcd_literals_candidate.py "$root/flash/Peristaltic_Pump_Software_v1.06/Peristaltic_Pump_Software_v1.06.ino"
cat > "$report/RESULT.txt" <<'EOF'
BASE_SOURCE_SHA256=e0d852757f74fedc4c00dd9da05b9cf02bf660c72bf7dcaa4afc52c8a7710e21
FLASH_SOURCE_SHA256=5c7009477ba56cb70850f22ece9860fcb222bd4c0a0c3a10622c21b66c79bfa5
EOF
getaddr() { awk -v wanted="$2" '$3==wanted {v=$1;sub(/^0080/,"",v);print v;exit}' "$1"; }
for entry in '16MHz arduino:avr:pro:cpu=16MHzatmega328 16000000' '8MHz arduino:avr:pro:cpu=8MHzatmega328 8000000'; do
  read -r clk fqbn hz <<< "$entry"
  for variant in base flash; do
    sketch="$root/$variant/Peristaltic_Pump_Software_v1.06"
    build="$root/build-$clk-$variant"
    arduino-cli compile --fqbn "$fqbn" --build-path "$build" "$sketch" > "$report/compile-$clk-$variant.log" 2>&1
    grep -E 'Sketch uses|Global variables use' "$report/compile-$clk-$variant.log" | tee -a "$report/RESULT.txt"
    elf=$(find "$build" -type f -name '*.elf' -print -quit)
    test -f "$elf"
    avr-size -A "$elf" > "$report/sections-$clk-$variant.txt"
    avr-nm -n -C "$elf" > "$report/symbols-$clk-$variant.txt"
    brk=$(getaddr "$report/symbols-$clk-$variant.txt" '__brkval')
    bss=$(getaddr "$report/symbols-$clk-$variant.txt" '__bss_end')
    lcd=$(getaddr "$report/symbols-$clk-$variant.txt" 'lcd_init_ok')
    menu=$(getaddr "$report/symbols-$clk-$variant.txt" 'menu_number_1')
    hw=$(getaddr "$report/symbols-$clk-$variant.txt" 'hwUiActive')
    test -n "$brk" && test -n "$bss"
    if [[ -z "$lcd" ]]; then lcd=0; fi
    if [[ -z "$menu" ]]; then menu=0; fi
    if [[ -z "$hw" ]]; then hw=0; fi
    timeout 45s "$root/stack_probe" "$elf" "$hz" "$brk" "$bss" "$lcd" "$menu" "$hw" > "$report/simavr-$clk-$variant.txt"
    grep -E 'SIMAVR_(SP_MIN|HEAP_BREAK_HWM|STACK_HEAP_GAP_OBSERVED|LCD_INIT_OBSERVED|INPUT_STIMULUS_STAGES)' "$report/simavr-$clk-$variant.txt" | tee -a "$report/RESULT.txt"
  done
done
python3 - "$report" <<'PY'
from pathlib import Path
import sys,re,json
report=Path(sys.argv[1]); output={}
for clk in ('16MHz','8MHz'):
    def parse(variant):
        log=(report/f'compile-{clk}-{variant}.log').read_text()
        ram=int(re.search(r'Global variables use (\d+) bytes',log).group(1))
        flash=int(re.search(r'Sketch uses (\d+) bytes',log).group(1))
        sim={}
        for line in (report/f'simavr-{clk}-{variant}.txt').read_text().splitlines():
            if '=' in line:
                k,v=line.split('=',1);sim[k]=v
        assert sim['SIMAVR_LCD_INIT_OBSERVED']=='1'
        assert int(sim['SIMAVR_INPUT_STIMULUS_STAGES'])==6
        assert int(sim['SIMAVR_I2C_WRITES'])>=100
        return dict(ram=ram,flash=flash,gap=int(sim['SIMAVR_STACK_HEAP_GAP_OBSERVED']),stack=int(sim['SIMAVR_STACK_DEPTH_OBSERVED']),lcd=sim['SIMAVR_LCD_INIT_OBSERVED'],i2c=int(sim['SIMAVR_I2C_WRITES']))
    base,flash=parse('base'),parse('flash')
    saved=base['ram']-flash['ram']; gapgain=flash['gap']-base['gap']
    print(f'{clk}: BASE={base}; FLASH={flash}; STATIC_SRAM_SAVED={saved}; BOUNDED_GAP_GAIN={gapgain}')
    if saved<32 or gapgain<16:
        raise SystemExit(f'REVIEW: {clk} gain too small; do not promote')
    output[clk]=dict(base=base,flash=flash,sram_saved=saved,bounded_gap_gain=gapgain)
(report/'QA_COMPARISON.json').write_text(json.dumps(output,indent=2))
with (report/'RESULT.txt').open('a') as f:
    f.write('AVR_2/2_FLASH_CANDIDATE=PASS\nBOUNDED_SIMAVR_AB_2/2=PASS\nALL_PATHS_STACK_HEAP_PEAK=REVIEW\nPHYSICAL_NOT_RUN;RELEASE_BLOCKED;NO_FIRMWARE_UPLOAD\n')
PY
