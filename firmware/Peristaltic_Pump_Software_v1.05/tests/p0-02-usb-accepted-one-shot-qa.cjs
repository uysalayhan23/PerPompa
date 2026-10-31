#!/usr/bin/env node
'use strict';
// P0-02 software acceptance (temporary candidate only): fail-closed. NO board upload.
// Native parser+dispatch PASS proofs are retained, not redundantly run here.
// This invokes a SHA-locked source gate, full real setup()/loop() host C++
// positive/negative tests for both F_CPU values, and two true Arduino AVR compiles.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto'),
      cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const expected='5b3c5a29ce8ce5eee0941469738b7e088d60a1bb';
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
 // Historical prototype and queue-order unit proofs are retained in prior
 // commits; current accepted firmware uses its own full-source 104-case QA.
 const host=cp.spawnSync(process.execPath,
   [path.join(__dirname,'p0-02-usb-accepted-source-qa.cjs'),'--host-only'],
   {encoding:'utf8',timeout:240000,maxBuffer:10*1024*1024,env:process.env});
 if(host.stdout)process.stdout.write(host.stdout);
 if(host.stderr)process.stderr.write(host.stderr);
 if(host.error||host.status!==0||
    !(host.stdout||'').includes('P002_USB_ACCEPTED_FULL_HOST_ONLY=PASS')||
    !(host.stdout||'').includes('FULL_SOURCE_HOST_16000000=PASS')||
    !(host.stdout||'').includes('FULL_SOURCE_HOST_8000000=PASS'))
  throw Error('P002_FULL_HOST_CXX_NOT_PASSED '+(host.error?.message||'exit '+host.status));
 check(cli,['version'],'ARDUINO_CLI_NOT_AVAILABLE');
 const result=cp.spawnSync(process.execPath,
   [path.join(__dirname,'p0-02-usb-accepted-source-qa.cjs'),'--avr-only'],
   {encoding:'utf8',timeout:240000,maxBuffer:10*1024*1024,env:process.env});
 if(result.stdout)process.stdout.write(result.stdout);
 if(result.stderr)process.stderr.write(result.stderr);
 if(result.error||result.status!==0||
    !(result.stdout||'').includes('P002_USB_ACCEPTED_AVR_ONLY=PASS')||
    !(result.stdout||'').includes('P002_USB_AVR_16MHzatmega328=PASS')||
    !(result.stdout||'').includes('P002_USB_AVR_8MHzatmega328=PASS'))
   throw Error('P002_FULL_LOOP_OR_AVR_NOT_PASSED '+(result.error?.message||'exit '+result.status));
 console.log('P002_USB_ACCEPTED_ONE_SHOT_SOFTWARE_PASS; FIRMWARE_BLOB_VERIFIED; HW=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_USB_ONE_SHOT=BLOCKED '+e.message);process.exitCode=2;}
