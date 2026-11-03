#!/usr/bin/env bash
# P004 QA-only: patched v1.6 simAVR at nominal 3.4ms, never hardware calibration.
set -euo pipefail
root="${RUNNER_TEMP:?}/p004-nominal-eepe"
mkdir -p "$root/report" "$RUNNER_TEMP/p004-marker-irq"
git clone -q --depth 1 --branch v1.6 https://github.com/buserror/simavr.git "$root/simavr"
revision=$(git -C "$root/simavr" rev-parse HEAD)
test "$revision" = '1b6e6ed10a92438da47fe99864698db9ac2169b6' || { echo "UNEXPECTED_SIMAVR_REVISION=$revision" >&2; exit 3; }
export P004_UPSTREAM_DIR="$root/simavr"
PYTHONPATH="$PWD/qa/p004" python3 -m unittest -v test_patch_simavr_eeprom | tee "$root/report/PATCH_TESTS.txt"
python3 qa/p004/patch_simavr_eeprom.py --upstream "$root/simavr" --dry-run
python3 qa/p004/patch_simavr_eeprom.py --upstream "$root/simavr"
make -C "$root/simavr/simavr" -j2 RELEASE=1 > "$root/report/simavr-build.txt" 2>&1
lib=$(find "$root/simavr/simavr" -name 'libsimavr.a' -type f -print -quit)
test -s "$lib" || { echo 'PATCHED_STATIC_LIBRARY_MISSING' >&2; exit 4; }
mkdir -p "$root/include"
ln -s "$root/simavr/simavr/sim" "$root/include/simavr"
# Explicit static link to patched simAVR. System libsimavr cannot be loaded.
gcc -std=c11 -O2 -Wall -Wextra -I "$root/include" -I "$root/simavr/simavr" \
  qa/p004/stack_integrated_marker_probe.c "$lib" -lelf -lm \
  -o "$RUNNER_TEMP/p004-marker-irq/stack_marker_irq_probe"
test -x "$RUNNER_TEMP/p004-marker-irq/stack_marker_irq_probe"
if ldd "$RUNNER_TEMP/p004-marker-irq/stack_marker_irq_probe" | grep -q libsimavr;then
  echo 'SYSTEM_SIMAVR_DYNAMIC_LINK_FORBIDDEN' >&2; exit 5
fi
printf 'SIMAVR_UPSTREAM_COMMIT=%s\nPATCHED_LIBRARY=%s\n' "$revision" "$lib" | tee -a "$root/report/RESULT.txt"
# Use original, SHA-locked v1.06 QA firmware, 16/8MHz, UART+Timer vector probes.
# This script must fail closed on any unavailable overlap or motor safety evidence.
if bash qa/p004/run_integrated_marker_avr.sh; then
  cp -a "$RUNNER_TEMP/p004-marker-irq/report/." "$root/report/"
else
  cp -a "$RUNNER_TEMP/p004-marker-irq/report/." "$root/report/"
  echo 'COMBINED_ISR_REVIEW: failed original code acceptance gate' >&2
  exit 6
fi
# Positive acceptance in this QA reference still does not mean physical calibration.
python3 - "$root/report" <<'PY'
from pathlib import Path
import json,sys
p=Path(sys.argv[1]);r=json.loads((p/'COMBINED_EEPROM_UART_TIMER_RESULTS.json').read_text())
for clk,minimum in [('16MHz',54400),('8MHz',27200)]:
  z=r[f'{clk}/POSITIVE']
  assert z['status']=='PASS',f'{clk}: original firmware ISR path not verified'
  d=z['data']
  begin=int(d['MARKER_OPEN_CYCLE']);end=int(d['MARKER_COMMIT_CYCLE'])
  assert end-begin>=minimum,f'{clk}: EEPROM timing compressed, not nominal'
  assert int(d['UART_ISR_DURING_WRITE'])>0 and int(d['TIMER_ISR_DURING_WRITE'])>0
  assert int(d['COMMITTED_STACK_HEAP_GAP'])>0
  assert int(d['MOTOR_STEP_HIGH_SAMPLES'])==0
  print(f'{clk}: NOMINAL_QA_ISR_OVERLAP_OBSERVED duration={end-begin} cycles, physical=NOT_RUN')
for clk in ('16MHz','8MHz'):
  assert r[f'{clk}/REMOTE_LOW']['status']=='PASS'
print('QA_NOMINAL_SIMAVR_4/4_PASS;PHYSICAL_NOT_RUN;RELEASE_BLOCKED;CALIBRATION_NOT_RUN')
PY
