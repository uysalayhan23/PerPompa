#!/usr/bin/env node
'use strict';
// P0-02 targeted native reproduction: source-identical pump/dose/Remote handlers.
// Models ONLY delayMicroseconds and debounce; not true AVR/oscilloscope timing.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),
      crypto=require('node:crypto'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const firmware=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
const harness=path.join(__dirname,'p0-02-targeted-source.cpp');
const sourceSHA='04185aaa799b0af0bfa663b8acc8f5013a515066';
const harnessSHA='0973ab25b16a3e041e5d2208dd757fe95683956f';
function sha(file){const b=fs.readFileSync(file);return crypto.createHash('sha1')
 .update('blob '+b.length).update(Buffer.from([0])).update(b).digest('hex');}
function body(s,sig){
 const start=s.indexOf(sig);if(start<0||s.indexOf(sig,start+sig.length)>=0)
  throw Error('MISSING_OR_AMBIGUOUS_SOURCE '+sig);
 let at=s.indexOf('{',start),depth=0;
 for(let i=at;i<s.length;i++){if(s[i]==='{')depth++;
 else if(s[i]==='}'&&!--depth)return s.slice(start,i+1);}
 throw Error('UNBALANCED_SOURCE '+sig);
}
let temp;
try {
 if(sha(firmware)!==sourceSHA||sha(harness)!==harnessSHA)
  throw Error('P002_ACCEPTED_FIRMWARE_OR_TEST_BLOB_CHANGED');
 const s=fs.readFileSync(firmware,'utf8'),h=fs.readFileSync(harness,'utf8');
 const names=['void pump(int _delay_us) {',
              'boolean dose(long _steps, int _delay_us, long & inc) {',
              'void handle_remote_input(){'];
 names.forEach(name=>{if(body(s,name)!==body(h,name))
     throw Error('REAL_SOURCE_FUNCTION_DIFFERENT '+name);});
 console.log('P002_SOURCE_BODY_IDENTITY=3/3 PASS');
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-p002-target-'));
 for(const fcpu of [16000000,8000000]){
  const exe=path.join(temp,'target-'+fcpu+(process.platform==='win32'?'.exe':''));
  const cmd=process.env.CXX||'g++';
  const build=cp.spawnSync(cmd,['-std=c++17','-O0','-Wall','-Wextra','-Werror',
   '-pedantic','-fsanitize=undefined','-fno-sanitize-recover=all',
   '-DF_CPU='+fcpu+'UL',harness,'-o',exe],
   {encoding:'utf8',timeout:60000,maxBuffer:2097152});
  if(build.stdout)process.stdout.write(build.stdout);
  if(build.stderr)process.stderr.write(build.stderr);
  if(build.error||build.status!==0)throw Error('CPP_BUILD_'+fcpu+' '+(build.error?.message||build.status));
  const run=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000,maxBuffer:2097152});
  if(run.stdout)process.stdout.write(run.stdout);
  if(run.stderr)process.stderr.write(run.stderr);
  if(run.error||run.status!==0||!run.stdout?.includes('P002_TARGETED_SOURCE_EXCERPT_CPP=8/8 PASS'))
   throw Error('CPP_REGRESSION_'+fcpu+' '+(run.error?.message||run.status));
 }
 console.log('P002_TARGETED_NATIVE_TWO_CPU_PASS=2/2; SOURCE_IDENTICAL; STOP_LATENCY_HW=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_TARGETED_NATIVE=BLOCKED '+e.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
