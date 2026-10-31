#!/usr/bin/env node
'use strict';
// Source-derived hot-path EEPROM workload gate; NOT an AVR cycle measurement.
// Fail CLOSED when a complete profile EEPROM reload occurs inside every STEP.
const fs=require('node:fs'),path=require('node:path');
const root=path.resolve(__dirname,'..');
const source=require('./hw-setup-01c-candidate.cjs').build();
const profile=fs.readFileSync(path.join(root,'hw-setup-profile.h'),'utf8');
const runtime=fs.readFileSync(path.join(root,'hw-setup-runtime.h'),'utf8');
function body(text,signature){
  const i=text.indexOf(signature);
  if(i<0 || text.indexOf(signature,i+signature.length)>=0)throw Error('SOURCE_ANCHOR_NOT_UNIQUE '+signature);
  const begin=text.indexOf('{',i);let depth=0;
  for(let j=begin;j<text.length;j++){
    if(text[j]==='{')depth++;
    if(text[j]==='}'&&!--depth)return text.slice(begin,j+1);
  }
  throw Error('UNBALANCED '+signature);
}
function count(s,regex){return (s.match(regex)||[]).length;}
try {
 const recordLength=Number(profile.match(/kRecordLength\s*=\s*(\d+)\s*;/)?.[1]);
 if(recordLength!==12)throw Error('PROFILE_RECORD_SIZE_CHANGED');
 const permit=body(runtime,'bool permit(StartPath path, bool stopRequested) const');
 const readLatest=body(runtime,'bool readLatest(Profile& p,uint8_t& gen,uint8_t& slot) const');
 const motor=body(source,'bool hwMotorAllowed()');
 const loop=body(source,'void loop()');
 const dose=body(source,'boolean dose(long _steps, int _delay_us, long & inc)');
 const pump=body(source,'void pump(int _delay_us)');
 const local=loop.slice(loop.indexOf('case 0: //Start'),loop.indexOf('case 8:'));
 const usb=loop.slice(loop.indexOf('case 9:'),loop.indexOf('/// MENU (no action)'));
 if(!motor.includes('hwSetup.permit(path,false)')||
    !permit.includes('readLatest(stored,gen,slot)')||
    !permit.includes('eeprom_.read(kSetupArmAddress)')||
    count(readLatest,/eeprom_\.read\(kSlot[01]Address\+i\)/g)!==2||
    !readLatest.includes('i<kRecordLength'))throw Error('PERMISSION_READ_PATH_CHANGED');
 const readsPerPermit=1+2*recordLength;
 const paths=[
   ['Local Pump',count(local,/hwMotorAllowed\(\)/g)+count(pump,/hwMotorAllowed\(\)/g)],
   ['Local Dose/Cal',count(local,/hwMotorAllowed\(\)/g)+count(dose,/hwMotorAllowed\(\)/g)],
   ['USB Pump',count(usb,/hwMotorAllowed\(\)/g)+count(pump,/hwMotorAllowed\(\)/g)],
   ['USB Dose/Cal',count(usb,/hwMotorAllowed\(\)/g)+count(dose,/hwMotorAllowed\(\)/g)]
 ];
 for(const [name,calls]of paths){
   if(calls<1)throw Error('MISSING_SAFETY_GATE '+name);
   console.log(name+': permission_checks_per_STEP='+calls+
               ' EEPROM_reads_per_authorized_STEP='+calls*readsPerPermit);
 }
 const maxReads=Math.max(...paths.map(x=>x[1]*readsPerPermit));
 console.log('P002_TIMING_SOURCE_REVIEW: EEPROM_READS_PER_AUTHORIZED_STEP='+maxReads+
             '; AVR_DURATION_US=NOT_RUN; STOP_LATENCY=REVIEW; HW_BENCH=NOT_RUN');
 if(maxReads>recordLength){
   console.error('HW_SETUP_01C_TIMING_GATE=BLOCKED: profile EEPROM reloads in motor STEP path');
   process.exitCode=2;
 }else{
   console.log('HW_SETUP_01C_TIMING_GATE=SOURCE_CHECK_PASS; STILL_REQUIRE_NATIVE_AND_AVR');
 }
}catch(e){
 console.error('HW_SETUP_01C_TIMING_GATE=BLOCKED '+e.message);
 process.exitCode=2;
}
