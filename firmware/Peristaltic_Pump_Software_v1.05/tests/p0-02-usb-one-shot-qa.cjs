#!/usr/bin/env node
'use strict';
// P0-02 software acceptance (temporary candidate only): fail-closed. NO board upload.
// Native parser+dispatch PASS proofs are retained, not redundantly run here.
// This invokes a SHA-locked source gate, full real setup()/loop() host C++
// positive/negative tests for both F_CPU values, and two true Arduino AVR compiles.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),
      cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const expected='04185aaa799b0af0bfa663b8acc8f5013a515066';
function blob(file){const b=fs.readFileSync(file);return crypto.createHash('sha1')
  .update('blob '+b.length).update(Buffer.from([0])).update(b).digest('hex');}
try{
 const ino=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
 if(blob(ino)!==expected)throw Error('APPROVED_FIRMWARE_SHA_CHANGED');
 const compiler=process.env.CXX||'g++';
 const cli=process.env.ARDUINO_CLI||'arduino-cli';
 const check=(cmd,args,label)=>{
  const run=cp.spawnSync(cmd,args,{encoding:'utf8',timeout:20000,maxBuffer:1048576});
  if(run.error||run.status!==0)throw Error(label+' '+(run.error?.message||run.stderr||run.status));
 };
 check(compiler,['--version'],'CXX_NOT_AVAILABLE');
 check(cli,['version'],'ARDUINO_CLI_NOT_AVAILABLE');
 const prototypeGate=cp.spawnSync(process.execPath,
   [path.join(__dirname,'p0-02-usb-prototype-order-gate.cjs')],
   {encoding:'utf8',timeout:60000,maxBuffer:1048576,env:process.env});
 if(prototypeGate.stdout)process.stdout.write(prototypeGate.stdout);
 if(prototypeGate.stderr)process.stderr.write(prototypeGate.stderr);
 if(prototypeGate.error||prototypeGate.status!==0||
    !(prototypeGate.stdout||'').includes('P002_PROTO_HEADER_NATIVE=2/2 PASS'))
   throw Error('P002_PROTO_HEADER_ORDER_NOT_PASSED '+(prototypeGate.error?.message||'exit '+prototypeGate.status));
 const queueGate=cp.spawnSync(process.execPath,
   [path.join(__dirname,'p0-02-usb-queue-order-gate.cjs')],
   {encoding:'utf8',timeout:60000,maxBuffer:1048576,env:process.env});
 if(queueGate.stdout)process.stdout.write(queueGate.stdout);
 if(queueGate.stderr)process.stderr.write(queueGate.stderr);
 if(queueGate.error||queueGate.status!==0||
    !(queueGate.stdout||'').includes('P002_QUEUE_ORDER_NATIVE=2/2 PASS'))
   throw Error('P002_USB_QUEUED_DATA_ORDER_NOT_PASSED '+(queueGate.error?.message||'exit '+queueGate.status));
 const result=cp.spawnSync(process.execPath,
   [path.join(__dirname,'p0-02-usb-full-loop-avr-qa.cjs')],
   {encoding:'utf8',timeout:240000,maxBuffer:10*1024*1024,env:process.env});
 if(result.stdout)process.stdout.write(result.stdout);
 if(result.stderr)process.stderr.write(result.stderr);
 if(result.error||result.status!==0||
    !(result.stdout||'').includes('P002_USB_CANDIDATE_FULL_QA=PASS')||
    !(result.stdout||'').includes('FULL_SOURCE_HOST_16000000=PASS')||
    !(result.stdout||'').includes('FULL_SOURCE_HOST_8000000=PASS')||
    !(result.stdout||'').includes('P002_USB_AVR_16MHzatmega328=PASS')||
    !(result.stdout||'').includes('P002_USB_AVR_8MHzatmega328=PASS'))
   throw Error('P002_FULL_LOOP_OR_AVR_NOT_PASSED '+(result.error?.message||'exit '+result.status));
 console.log('P002_USB_ONE_SHOT_SOFTWARE_PASS; ACTIVE_FIRMWARE_SHA_UNCHANGED; HW=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_USB_ONE_SHOT=BLOCKED '+e.message);process.exitCode=2;}
