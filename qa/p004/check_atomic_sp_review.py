#!/usr/bin/env python3
"""P0-04 positive/negative QA for atomic SPH/SPL instrumentation (not hardware release)."""
from pathlib import Path
import json,re,sys

def parse(text):
    data={}
    for ln in text.splitlines():
        if "=" in ln:
            k,v=ln.split("=",1)
            if k.startswith("SP_") or k.startswith("SIMAVR_"):data[k]=v
    keys=("SP_RAW_LOW","SP_COMMITTED_LOW","SP_SPH_WRITES","SP_SPL_WRITES",
          "SP_PARTIAL_SAMPLES","SP_UNPAIRED","SP_COMMITTED_STACK_DEPTH_OBSERVED",
          "SP_COMMITTED_HEAP_GAP_OBSERVED","SIMAVR_HEAP_BREAK_HWM",
          "SIMAVR_STACK_HEAP_GAP_OBSERVED","SIMAVR_STACK_DEPTH_OBSERVED")
    assert all(k in data for k in keys), "Incomplete trace"
    raw=int(data["SP_RAW_LOW"],16)
    committed=int(data["SP_COMMITTED_LOW"],16)
    heap=int(data["SIMAVR_HEAP_BREAK_HWM"],16)
    assert 0x100<raw<committed<=0x08ff, "SPH-only sample not distinguished"
    assert committed-raw >= 16, "No substantive partial-SP gap observed"
    assert int(data["SP_UNPAIRED"])==0, "Unpaired or too-long SPH/SPL update"
    assert int(data["SP_SPH_WRITES"])>=1, "No SPH writes detected"
    assert int(data["SP_SPL_WRITES"])>=1, "No SPL writes detected"
    assert int(data["SP_PARTIAL_SAMPLES"])>=1, "No partial SP sample"
    assert int(data["SIMAVR_STACK_HEAP_GAP_OBSERVED"]) == raw-heap
    assert int(data["SP_COMMITTED_HEAP_GAP_OBSERVED"]) == committed-heap
    assert int(data["SP_COMMITTED_STACK_DEPTH_OBSERVED"]) == 0x08ff-committed
    assert int(data["SIMAVR_STACK_DEPTH_OBSERVED"]) == 0x08ff-raw
    assert int(data["SP_COMMITTED_HEAP_GAP_OBSERVED"])>0, "Observed heap collision"
    assert int(data["SIMAVR_LCD_INIT_OBSERVED"])==1
    assert int(data["SIMAVR_INPUT_STIMULUS_STAGES"])==6
    assert int(data["SIMAVR_I2C_WRITES"])>100
    return data

def raw_min_pc(text):
    lines=[x for x in text.splitlines() if x.startswith("TRACE_LOW=")]
    assert lines
    last=lines[-1].split("=",1)[1].split(",")
    assert len(last)==5
    return int(last[2],16),int(last[1],16)

def disasm_check(disasm, next_pc):
    def at(addr):
        match=re.search(r"^\s*"+f"{addr:x}"+r":\s+[^\n]*$",disasm,re.M)
        assert match, f"No opcode at {addr:x}"
        return match.group()
    # A transient 0x100 SP drop was captured immediately after OUT SPH.
    sph=at(next_pc-2)
    spl=at(next_pc+2)
    assert re.search(r"\bout\s+0x3e,",sph),f"Not OUT SPH: {sph}"
    assert re.search(r"\bout\s+0x3d,",spl),f"Not OUT SPL: {spl}"
    return sph,spl

def negatives(src,disasm,pc,dat):
    cases=[]
    altered=[
        ("raw_passed_as_committed",src.replace("SP_COMMITTED_LOW="+dat["SP_COMMITTED_LOW"],
            "SP_COMMITTED_LOW="+dat["SP_RAW_LOW"],1)),
        ("unpaired_sph",src.replace("SP_UNPAIRED=0","SP_UNPAIRED=1",1)),
        ("no_sph",src.replace("SP_SPH_WRITES="+dat["SP_SPH_WRITES"],"SP_SPH_WRITES=0",1)),
        ("wrong_heap_gap",src.replace("SP_COMMITTED_HEAP_GAP_OBSERVED="+dat["SP_COMMITTED_HEAP_GAP_OBSERVED"],
            "SP_COMMITTED_HEAP_GAP_OBSERVED=1",1)),
    ]
    for label,bad in altered:
        assert bad != src
        try:parse(bad)
        except (AssertionError,ValueError,KeyError):cases.append(label)
        else:raise AssertionError("Negative SP detector passed: "+label)
    sph,spl=disasm_check(disasm,pc)
    bad_disasm=disasm.replace(sph,sph.replace("out","nop",1),1)
    assert bad_disasm!=disasm
    try:disasm_check(bad_disasm,pc)
    except (AssertionError,ValueError):cases.append("bad_sph_opcode")
    else:raise AssertionError("Bad machine code not rejected")
    assert len(cases)==5
    return cases

def main():
    report=Path(sys.argv[1]);results={}
    for clock in ("16MHz","8MHz"):
        raw=(report/f"simavr-{clock}.txt").read_text()
        disasm=(report/f"disasm-{clock}.txt").read_text(errors="replace")
        data=parse(raw)
        pc,sp=raw_min_pc(raw)
        assert sp==int(data["SP_RAW_LOW"],16)
        sph,spl=disasm_check(disasm,pc)
        negatives_detected=negatives(raw,disasm,pc,data)
        row={
          "clock":clock,
          "raw_sp":data["SP_RAW_LOW"],
          "committed_sp":data["SP_COMMITTED_LOW"],
          "raw_stack_depth":int(data["SIMAVR_STACK_DEPTH_OBSERVED"]),
          "committed_stack_depth":int(data["SP_COMMITTED_STACK_DEPTH_OBSERVED"]),
          "raw_gap":int(data["SIMAVR_STACK_HEAP_GAP_OBSERVED"]),
          "committed_gap":int(data["SP_COMMITTED_HEAP_GAP_OBSERVED"]),
          "raw_low_pc":f"0x{pc:x}",
          "committed_low_pc":data.get("SP_COMMITTED_PC",""),
          "SPH_writes":int(data["SP_SPH_WRITES"]),
          "SPL_writes":int(data["SP_SPL_WRITES"]),
          "partial_samples":int(data["SP_PARTIAL_SAMPLES"]),
          "negative_checks":negatives_detected,
          "machine_code_evidence":{"SPH":sph.strip(),"SPL":spl.strip()}
        }
        results[clock]=row
        print(f"{clock}: RAW sp={row['raw_sp']} stack={row['raw_stack_depth']}B gap={row['raw_gap']}B; COMMITTED sp={row['committed_sp']} stack={row['committed_stack_depth']}B gap={row['committed_gap']}B; SPH={row['SPH_writes']} SPL={row['SPL_writes']} partial_samples={row['partial_samples']}")
        print(f"{clock}: PC {row['raw_low_pc']} OUT SPH then OUT SPL, negative QA 5/5 rejected")
    (report/"ATOMIC_SP_VERIFICATION.json").write_text(json.dumps(results,indent=2))
    print("ATOMIC_SP_2/2_POSITIVE_PASS;10/10_NEGATIVE_REJECT;NOT_HARDWARE_PEAK")
    print("ISR_ALL_PATHS_STACK=REVIEW;PHYSICAL_NOT_RUN;RELEASE_BLOCKED")
if __name__=="__main__":main()
