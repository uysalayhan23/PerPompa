#!/usr/bin/env bash
# QA-only synthetic local menu action; real AVR EEPROM/Timer/UART code.
set -euo pipefail
root="$RUNNER_TEMP/p004-marker-irq"; report="$root/report"
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
    if timeout 55s "$root/stack_marker_irq_probe" "$elf" "$hz" \
      "${args[0]}" "${args[1]}" "${args[2]}" "${args[3]}" "${args[4]}" \
      "${args[5]}" "${args[6]}" "${args[7]}" "${args[8]}" "${args[9]}" "$negative" \
       > "$report/avr-$clk-$scenario.txt" 2>&1; then
      echo "SIM_STATUS_$clk-$scenario=ACCEPTED" | tee -a "$report/RESULT.txt"
    else
      echo "SIM_STATUS_$clk-$scenario=REVIEW_FAILED" | tee -a "$report/RESULT.txt"
    fi
    grep -E '^(QA_SCENARIO|QA_ACCEPT|COMMITTED_STACK_DEPTH|COMMITTED_STACK_HEAP_GAP|UART_ISR_DURING_WRITE|TIMER_ISR_DURING_WRITE|SIMAVR_EEPROM_COMMITTED|NEGATIVE_EEPROM_VIOLATIONS|MOTOR_STEP_HIGH_SAMPLES)=' "$report/avr-$clk-$scenario.txt" | tee -a "$report/RESULT.txt"
  done
done
python3 - "$report" <<'PY'
from pathlib import Path
import json,sys
root=Path(sys.argv[1]);results={};failures=[]
def check(v,clock,scenario):
    err=[]
    def requires(condition,label):
        if not condition: err.append(label)
    def number(key):
        try:return int(v[key])
        except (ValueError,KeyError,TypeError):return -999999
    positive=scenario=="POSITIVE"
    requires(v.get("QA_SCENARIO")==("LEGACY_FIRST_SAVE_WITH_CONCURRENT_ISR" if positive else "REMOTE_LOW_BLOCKED"),"scenario identity")
    requires(v.get("QA_ACCEPT")=="1","simulator acceptance")
    requires(number("UART_ISR_ENTRY_COUNT")>0,"UART RX handler never executed")
    requires(number("TIMER_ISR_ENTRY_COUNT")>0,"Timer1 handler never executed")
    requires(number("COMMITTED_STACK_HEAP_GAP")>0,"stack/heap no gap")
    requires(number("SPH_WRITES")>0 and number("SPH_WRITES")==number("SPL_WRITES"),"unpaired SPH/SPL count")
    requires(number("SP_UNPAIRED")==0,"incomplete SPH/SPL update")
    requires(number("MOTOR_STEP_HIGH_SAMPLES")==0,"motor STEP was high")
    requires(number("SIMAVR_UART_RX_STIMULI")==7,"UART input count wrong")
    if positive:
        begin=number("MARKER_OPEN_CYCLE");end=number("MARKER_COMMIT_CYCLE")
        uart=number("FIRST_UART_ISR_DURING_CYCLE")
        timer=number("FIRST_TIMER_ISR_DURING_CYCLE")
        requires(number("MARKER_ZERO_SEEN")==1,"slot marker zero not observed")
        requires(begin>0 and end>begin,"no valid EEPROM marker-to-commit window")
        requires(number("SIMAVR_EEPROM_WITNESS_OBSERVED")==1 and number("SIMAVR_EEPROM_COMMITTED")==1,"EEPROM not committed")
        requires(number("UART_ISR_DURING_WRITE")>0,"UART ISR absent in save window")
        requires(number("TIMER_ISR_DURING_WRITE")>0,"Timer ISR absent in save window")
        requires(begin <= uart < end,"UART ISR timestamp outside save window")
        requires(begin <= timer < end,"Timer ISR timestamp outside save window")
    else:
        requires(number("MARKER_ZERO_SEEN")==0,"unsafe Remote LOW caused slot invalidation")
        requires(number("SIMAVR_EEPROM_WITNESS_OBSERVED")==0 and number("SIMAVR_EEPROM_COMMITTED")==0,"unsafe Remote LOW stored data")
        requires(number("NEGATIVE_EEPROM_VIOLATIONS")==0,"unsafe EEPROM writes")
    return err
for clock in ("16MHz","8MHz"):
 for scenario in ("POSITIVE","REMOTE_LOW"):
    path=root/f"avr-{clock}-{scenario}.txt"
    if not path.exists():
        failures.append(f"{clock}/{scenario}: missing simulator output");continue
    v=dict(line.split("=",1) for line in path.read_text().splitlines() if "=" in line)
    err=check(v,clock,scenario)
    info={k:v.get(k) for k in ("QA_ACCEPT","UART_ISR_ENTRY_COUNT","TIMER_ISR_ENTRY_COUNT","UART_ISR_DURING_WRITE","TIMER_ISR_DURING_WRITE","MARKER_ZERO_SEEN","MARKER_OPEN_CYCLE","MARKER_COMMIT_CYCLE","FIRST_UART_ISR_DURING_CYCLE","FIRST_TIMER_ISR_DURING_CYCLE","SIMAVR_EEPROM_COMMITTED","COMMITTED_STACK_HEAP_GAP","MOTOR_STEP_HIGH_SAMPLES")}
    results[f"{clock}/{scenario}"]={"data":info,"status":"PASS" if not err else "REVIEW","errors":err}
    print(f"{clock}/{scenario}: {'PASS' if not err else 'REVIEW'}: {info} | {err}")
    for e in err:failures.append(f"{clock}/{scenario}: {e}")
    if not err:
        tampered=v.copy()
        tampered["MOTOR_STEP_HIGH_SAMPLES"]="1"
        assert check(tampered,clock,scenario), "unsafe motor-output mutation escaped negative test"
        tampered=v.copy()
        tampered["COMMITTED_STACK_HEAP_GAP"]="0"
        assert check(tampered,clock,scenario), "stack overlap mutation escaped negative test"
        tampered=v.copy()
        if scenario=="POSITIVE":
            tampered["UART_ISR_DURING_WRITE"]="0"
            assert check(tampered,clock,scenario),"missing overlap mutation escaped negative test"
        else:
            tampered["MARKER_ZERO_SEEN"]="1"
            assert check(tampered,clock,scenario),"Remote LOW EEPROM write mutation escaped negative test"
(root/"COMBINED_EEPROM_UART_TIMER_RESULTS.json").write_text(json.dumps(results,indent=2))
if failures:
    print("QA_BOUNDED_COMBINED_ISR=REVIEW_FAIL",failures)
    raise SystemExit(6)
print("QA_COMBINED_4/4_PASS;NEGATIVE_PARSER_MUTANTS_12/12_REJECT")
print("ONLY_SYNTHETIC_SIMAVR;ALL_PATHS_STACK_ISR_REVIEW;PHYSICAL_NOT_RUN;RELEASE_BLOCKED")
PY
printf '%s\n' 'QA_ONLY_SIMAVR_COMBINED_PATH_SUCCESS' 'NO_REAL_DEVICE;PRODUCTION_UNCHANGED;PHYSICAL_NOT_RUN;RELEASE_BLOCKED' | tee -a "$report/RESULT.txt"
