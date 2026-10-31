#!/usr/bin/env node
'use strict';
// Read-only dual-target Arduino compiler gate. Never modifies repo or flashes a board.
const fs=require('node:fs'),os=require('node:os'),path=require('node:path');
const cp=require('node:child_process'),crypto=require('node:crypto');
const repo=path.resolve(__dirname,'../../..');
const sketch=path.join(repo,'firmware/Peristaltic_Pump_Software_v1.05');
const active=path.join(sketch,'Peristaltic_Pump_Software_v1.05.ino');
const baseline=path.join(repo,'firmware/archive/Peristaltic_Pump_Software_v1.04/Peristaltic_Pump_Software_v1.04.ino');
function gitBlob(f) {
 const b=fs.readFileSync(f);
 return crypto.createHash('sha1').update(Buffer.from('blob '+b.length+'\0')).update(b).digest('hex');
}
function invoke(tool,args){
 return cp.spawnSync(tool,args,{encoding:'utf8',timeout:120000,maxBuffer:2*1024*1024});
}
function blocked(reason){
 console.error(reason);
 console.error('AVR_ARDUINO_BUILD=NOT_RUN_OR_FAIL | P0-01_SOURCE=BLOCKED | HW_BENCH=NOT_RUN');
 process.exitCode=2;
}
const expected='0d965c08947ba0b3170d22fbff806fc2e6f1e83a';
if(gitBlob(baseline)!==expected) {
 blocked('v1.04 immutable archive hash mismatch');
} else if (!fs.existsSync(active)) {
 blocked('Active v1.05 firmware missing');
} else {
 console.log('ARCHIVE_SHA=PASS | ACTIVE_BLOB='+gitBlob(active));
 const bin=process.env.ARDUINO_CLI||'arduino-cli';
 const check=invoke(bin,['version']);
 if(check.error||check.status!==0) {
  blocked('Arduino CLI unavailable: '+(check.error?.message||check.stderr));
 } else {
  console.log(check.stdout.trim());
  const temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-dual-build-'));
  try{
   let passed=0;
   for(const cpu of ['16MHzatmega328','8MHzatmega328']){
    const fqbn='arduino:avr:pro:cpu='+cpu;
    const cmd=invoke(bin,['compile','--fqbn',fqbn,'--build-path',path.join(temp,cpu),sketch]);
    if(cmd.error||cmd.status!==0) {
     blocked('AVR_ARDUINO_BUILD_FAIL '+fqbn+': '+(cmd.error?.message||cmd.stderr||cmd.stdout));
     break;
    }
    passed++;
    console.log('AVR_ARDUINO_BUILD_PASS '+fqbn);
   }
   if(passed===2)console.log('DUAL_BUILD_PASS=2/2 | HW_BENCH=NOT_RUN | RELEASE_BLOCKED');
  }finally{fs.rmSync(temp,{recursive:true,force:true});}
 }
}
