#!/usr/bin/env node
'use strict';
// Queue-first processing regression. AVR and HW acceptance are separate.
// The negative native probe confirms tick-first is unsafe on queued bytes.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),crypto=require('node:crypto'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const cpp=path.join(__dirname,'p0-02-usb-queue-order-native.cpp');
const header=path.join(root,'p0-02-usb-parser.h');
const ino=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
const expectedCpp='b1d1bbb8f3dc89f736c01a72cbb1934016622328';
const expectedHeader='20527923030414d7d385b5102b9307446ce7f5c7';
const expectedFirmware='04185aaa799b0af0bfa663b8acc8f5013a515066';
function gitBlob(file){const bytes=fs.readFileSync(file);return crypto.createHash('sha1')
 .update('blob '+bytes.length).update(Buffer.from([0])).update(bytes).digest('hex');}
let temp;
try{
 if(gitBlob(cpp)!==expectedCpp||gitBlob(header)!==expectedHeader||gitBlob(ino)!==expectedFirmware)
   throw Error('P002_QUEUE_GUARD_SHA_MISMATCH');
 const {build}=require('./p0-02-usb-candidate.cjs');
 const src=build(),usb=src.slice(src.indexOf('  case 9: {'),src.indexOf('/// MENU (no action)'));
 const feed=usb.indexOf('usbParser.feed((uint8_t)Serial.read(),millis());');
 const tick=usb.indexOf('usbParser.tick(millis());');
 if(feed<0||tick<=feed||!usb.includes('if (!usbDropUntilEmpty && !Serial.available()) {'))
   throw Error('P002_PREMATURE_QUIET_TICK_PRESENT');
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-queue-first-'));
 for(const hz of [16000000,8000000]){
   const exe=path.join(temp,'native-'+hz+(process.platform==='win32'?'.exe':''));
   const args=['-std=c++11','-O0','-Wall','-Wextra','-Werror','-pedantic',
    '-fsanitize=undefined','-fno-sanitize-recover=all','-DF_CPU='+hz+'UL','-I',root,cpp,'-o',exe];
   const compile=cp.spawnSync(process.env.CXX||'g++',args,{encoding:'utf8',timeout:60000});
   if(compile.stdout)process.stdout.write(compile.stdout);
   if(compile.stderr)process.stderr.write(compile.stderr);
   if(compile.error||compile.status!==0)
     throw Error('CPP_BUILD_'+hz+' '+(compile.error?.message||compile.status));
   const run=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000});
   if(run.stdout)process.stdout.write(run.stdout);
   if(run.stderr)process.stderr.write(run.stderr);
   if(run.error||run.status!==0||
      !(run.stdout||'').includes('P002_QUEUE_ORDER_NATIVE_FCPU_'+hz+'=13/13 PASS'))
     throw Error('CPP_RUN_'+hz+' '+(run.error?.message||run.status));
 }
 console.log('P002_QUEUE_ORDER_NATIVE=2/2 PASS; OLD_SOURCE_ORDER_REPRODUCED; ACTIVE_INO_UNCHANGED; FULL_LOOP_CPP=NOT_RUN; AVR=NOT_RUN; HW=NOT_RUN');
}catch(e){console.error('P002_QUEUE_ORDER_NATIVE=BLOCKED '+e.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
