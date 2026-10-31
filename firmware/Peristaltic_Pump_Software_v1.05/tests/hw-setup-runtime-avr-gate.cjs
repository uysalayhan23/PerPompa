#!/usr/bin/env node
'use strict';
// Compile-only probe for the independent HW Setup adapter. NOT full v1.05 integration.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'..'),active=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
const profile=path.join(root,'hw-setup-profile.h'),adapter=path.join(root,'hw-setup-runtime.h');
const approved='6617cc38bde9bddf1c32cd0506915c0054556127';
function gitSha(file){const b=fs.readFileSync(file);return crypto.createHash('sha1').update('blob '+b.length+'\0').update(b).digest('hex');}
let tmp=null;
try{
 if(!fs.existsSync(active)||!fs.existsSync(profile)||!fs.existsSync(adapter)||gitSha(active)!==approved)throw Error('P0_01_SOURCE_SHA_MISMATCH');
 tmp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-hw-avr-'));
 const folder=path.join(tmp,'PerPompa_HW_Setup_Probe');fs.mkdirSync(folder);
 for(const name of ['hw-setup-profile.h','hw-setup-runtime.h'])fs.copyFileSync(path.join(root,name),path.join(folder,name));
 fs.writeFileSync(path.join(folder,'PerPompa_HW_Setup_Probe.ino'),[
   '#include <EEPROM.h>',
   '#include "hw-setup-runtime.h"',
   'perpompa_hw::SafeSetup<EEPROMClass> hw(EEPROM,F_CPU);',
   'void setup(){hw.boot(); char row1[17],row2[17]; hw.lcdRows(row1,row2);}',
   'void loop(){volatile bool can=hw.permit(perpompa_hw::kLocalDose,false);(void)can;}',
   ''
 ].join('\n'));
 const cli=process.env.ARDUINO_CLI||'arduino-cli';
 for(const cpu of ['16MHzatmega328','8MHzatmega328']){
   const fqbn='arduino:avr:pro:cpu='+cpu;
   const r=cp.spawnSync(cli,['compile','--fqbn',fqbn,'--build-path',path.join(tmp,'build-'+cpu),folder],{encoding:'utf8',timeout:180000,maxBuffer:4194304});
   if(r.stdout)process.stdout.write(r.stdout);if(r.stderr)process.stderr.write(r.stderr);
   if(r.error||r.status!==0)throw Error('AVR_COMPILE_'+cpu+': '+(r.error?.message||r.status));
   console.log('HW_SETUP_RUNTIME_AVR_PROBE_PASS '+fqbn);
 }
 console.log('HW_SETUP_RUNTIME_AVR_PROBE=2/2 PASS; ORIGINAL_INO_INTEGRATION=NOT_RUN; HW=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('HW_SETUP_RUNTIME_AVR_PROBE=BLOCKED: '+e.message);process.exitCode=2;}
finally{if(tmp)fs.rmSync(tmp,{recursive:true,force:true});}
