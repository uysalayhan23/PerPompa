#!/usr/bin/env node
'use strict';
// P0-02 combined source and scoped C++ review. It NEVER marks HW STOP safe,
// compiles no AVR candidate, modifies no firmware, and flashes no board.
const path=require('node:path'),cp=require('node:child_process');
const tests=[
 ['p0-02-source-audit.cjs','P002_SOURCE_AUDIT=PASS'],
 ['p0-02-usb-blocking-negative.cjs','P002_USB_BLOCKING_NEGATIVE_SOURCE_REPRO=PASS'],
 ['p0-02-targeted-source-gate.cjs','P002_TARGETED_NATIVE_TWO_CPU_PASS=2/2'],
 ['p0-02-warning-native-gate.cjs','P002_WARNING_TARGET_NATIVE_TWO_CPU=2/2 PASS']
];
for(const [name,marker] of tests){
 const run=cp.spawnSync(process.execPath,[path.join(__dirname,name)],
   {encoding:'utf8',timeout:180000,maxBuffer:4*1024*1024,env:process.env});
 if(run.stdout)process.stdout.write(run.stdout);
 if(run.stderr)process.stderr.write(run.stderr);
 if(run.error||run.status!==0||!(run.stdout||'').includes(marker)){
  console.error('P002_ONE_SHOT=BLOCKED name='+name+
      ' error='+(run.error?.message||'exit '+run.status)+' expected='+marker);
  process.exit(2);
 }
}
console.log('P002_REVIEW_SOURCE_AND_SCOPED_CPP=PASS; FIRMWARE_UNCHANGED; AVR_NEW_CANDIDATE=NOT_RUN');
console.log('P002_STOP_LATENCY=REVIEW; NONBLOCKING_USB_FIX=NOT_RUN; HW_BENCH=NOT_RUN; RELEASE_BLOCKED');
