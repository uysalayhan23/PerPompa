#!/usr/bin/env python3
"""QA only. Attribute bounded simAVR SP records to optimized AVR ELF symbols.
This is NOT an all-path call-stack proof and never programs real hardware.
"""
from pathlib import Path
import bisect
import json
import re
import sys

def symbols_from_nm(path):
    rows = []
    for line in path.read_text(errors="replace").splitlines():
        m = re.match(r"^([0-9a-fA-F]{8})\s+([tTwW])\s+(.+)$", line)
        if m:
            addr = int(m.group(1), 16)
            if 0 <= addr < 32768:
                rows.append((addr, m.group(3)))
    rows.sort()
    assert rows, "No AVR text symbols; do not attribute"
    return rows

def resolve(rows, pc):
    assert isinstance(pc,int) and 0 <= pc < 32768, "PC outside AVR program flash"
    addresses=[x[0] for x in rows]
    index=bisect.bisect_right(addresses,pc)-1
    assert index >= 0, "No preceding symbol: no attribution"
    return rows[index][1],pc-rows[index][0]

def parse_trace(src):
    p={}
    lines=[]
    phases={}
    for line in src.splitlines():
        if "=" not in line: continue
        k,v=line.split("=",1)
        if k=="TRACE_LOW":
            fields=v.split(",")
            assert len(fields)==5
            idx,sp,pc,cycle,stage=fields
            lines.append(dict(i=int(idx),sp=int(sp,16),pc=int(pc,16),
                              cycle=int(cycle),stage=int(stage)))
        elif k=="TRACE_PHASE":
            stage,sp=v.split(",")
            phases[int(stage)]=int(sp,16)
        else:
            p[k]=v
    count=int(p["TRACE_TOTAL"]); kept=int(p["TRACE_KEPT"])
    assert count==kept==len(lines),"Truncated or missing SP trace"
    assert len(lines)>1
    assert all(x["i"]==i for i,x in enumerate(lines))
    assert all(0x100 <= x["sp"] <= 0x8ff for x in lines),"Invalid AVR SRAM pointer"
    assert all(x["sp"]>lines[i+1]["sp"] for i,x in enumerate(lines[:-1])), "SP trace not strictly decreasing"
    assert lines[-1]["sp"]==int(p["SIMAVR_SP_MIN"],16),"Trace final high-water does not match SP"
    assert int(p["SIMAVR_INPUT_STIMULUS_STAGES"])==6,"Input trace incomplete"
    assert int(p["SIMAVR_LCD_INIT_OBSERVED"])==1,"LCD not initialized"
    assert int(p["SIMAVR_I2C_WRITES"])>100,"No LCD I2C traffic"
    assert len(phases)==7 and set(phases)==set(range(7)),"Missing phase minima"
    assert int(p["SIMAVR_STACK_HEAP_GAP_OBSERVED"])>0,"Observed SRAM overlap"
    return p,lines,phases

def negative_controls(rows, src):
    p,events,phases=parse_trace(src)
    assert resolve(rows,events[-1]["pc"])[0]
    cases=[]
    for label,mutator in (
        ("invalid_pc",lambda s:s.replace("TRACE_LOW="+str(events[-1]["i"])+",0x", "TRACE_LOW="+str(events[-1]["i"])+",0x",1)),
        ("missing_trace",lambda s:s.replace("TRACE_TOTAL="+p["TRACE_TOTAL"],"TRACE_TOTAL=0",1)),
        ("missing_lcd",lambda s:s.replace("SIMAVR_LCD_INIT_OBSERVED=1","SIMAVR_LCD_INIT_OBSERVED=0",1)),
        ("missing_stage",lambda s:s.replace("SIMAVR_INPUT_STIMULUS_STAGES=6","SIMAVR_INPUT_STIMULUS_STAGES=5",1)),
    ):
        if label=="invalid_pc":
            try:resolve(rows,0xffffffff)
            except AssertionError:cases.append(label)
            else:raise AssertionError("Out-of-flash PC was attributed")
        else:
            mutated=mutator(src)
            assert mutated!=src,"Negative mutation did not affect transcript"
            try:parse_trace(mutated)
            except (AssertionError,ValueError,KeyError):cases.append(label)
            else:raise AssertionError("Undetected corrupted trace: "+label)
    assert len(cases)==4
    return cases

def main():
    report=Path(sys.argv[1])
    results={}
    for clock in ("16MHz","8MHz"):
        pth=report/f"simavr-{clock}.txt"
        sym=report/f"symbols-{clock}.txt"
        raw=pth.read_text()
        rows=symbols_from_nm(sym)
        p,records,phases=parse_trace(raw)
        negatives=negative_controls(rows,raw)
        for e in records:
            e["symbol"],e["offset"]=resolve(rows,e["pc"])
        final=records[-1]
        print(f"{clock}: highwater_sp=0x{final['sp']:04x} pc=0x{final['pc']:04x} symbol={final['symbol']}+0x{final['offset']:x} stage={final['stage']} min_gap={p['SIMAVR_STACK_HEAP_GAP_OBSERVED']}B trace_events={len(records)}")
        print(f"{clock}: last high-water entries:")
        for e in records[-12:]:
            print(f"  SP=0x{e['sp']:04x} PC=0x{e['pc']:04x} {e['symbol']}+0x{e['offset']:x} stage={e['stage']} cycle={e['cycle']}")
        print(f"{clock}: host negative controls {len(negatives)}/4 EXPECTED_REJECT")
        results[clock]=dict(final=final,first=records[0],trace_events=len(records),phase_sp=phases,tail=records[-32:],
                            negative_controls=negatives,observed_stack_bytes=int(p["SIMAVR_STACK_DEPTH_OBSERVED"]),
                            observed_gap_bytes=int(p["SIMAVR_STACK_HEAP_GAP_OBSERVED"]),
                            source_scope=p.get("SIMAVR_SCOPE",""))
    (report/"SP_ORIGIN_ANALYSIS.json").write_text(json.dumps(results,indent=2))
    print("QA_PC_SYMBOL_ATTRIBUTION_2/2_PASS;NEGATIVE_CONTROLS_8/8_EXPECTED_REJECT")
    print("ALL_PATHS_ISR_STACK_BOUND=NOT_RUN;PHYSICAL_NOT_RUN;RELEASE_BLOCKED")
if __name__=="__main__": main()
