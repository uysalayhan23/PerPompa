#!/usr/bin/env node
'use strict';
// Read-only quality gate for a *temporary* P0-01 R001/R007 candidate.
// Does not flash a board, modify firmware, or certify industrial release.
const fs=require('node:fs'),path=require('node:path'),cp=require('node:child_process'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'../../..');
const sketch=path.join(root,'firmware/Peristaltic_Pump_Software_v1.05/Peristaltic_Pump_Software_v1.05.ino');
const archive=path.join(root,'firmware/archive/Peristaltic_Pump_Software_v1.04/Peristaltic_Pump_Software_v1.04.ino');
const expected='0d965c08947ba0b3170d22fbff806fc2e6f1e83a';
function sha(file){const b=fs.readFileSync(file);return crypto.createHash('sha1').update('blob '+b.length+'\0').update(b).digest('hex');}
function cli(){
 if(process.env.ARDUINO_CLI)return process.env.ARDUINO_CLI;
 const bin=process.platform==='win32'?'arduino-cli.exe':'arduino-cli';
 const r=cp.spawnSync(bin,['version'],{encoding:'utf8',timeout:10000});
 if(!r.error && r.status===0)return bin;
 if(process.platform==='win32' && process.env.LOCALAPPDATA){
  const q=[path.join(process.env.LOCALAPPDATA,'ArduinoCLI')];
  for(let n=0;q.length&&n<150;n++){
   const dir=q.shift();if(!fs.existsSync(dir))continue;
   for(const e of fs.readdirSync(dir,{withFileTypes:true})){
    const p=path.join(dir,e.name);
    if(e.isFile()&&e.name.toLowerCase()==='arduino-cli.exe')return p;
    if(e.isDirectory())q.push(p);
   }
  }
 }
 throw Error('Arduino CLI unavailable; supply ARDUINO_CLI path');
}
function check(label,script,markers,env){
 console.log('\n'+label);
 const r=cp.spawnSync(process.execPath,[path.join(__dirname,script)],{cwd:root,env,encoding:'utf8',timeout:240000,maxBuffer:5*1024*1024});
 const out=(r.stdout||'')+'\n'+(r.stderr||'');
 if(r.stdout)process.stdout.write(r.stdout);
 if(r.stderr)process.stderr.write(r.stderr);
 if(r.error||r.status!==0||!markers.every(x=>out.includes(x)))throw Error(label+' blocked: '+(r.error?.message||'exit '+r.status));
 console.log(label+'=PASS');
}
try {
 if(!fs.existsSync(sketch)||!fs.existsSync(archive)||sha(sketch)!==expected||sha(archive)!==expected)
  throw Error('v1.05 or immutable v1.04 Git blob SHA mismatch');
 const cliPath=cli();
 const ver=cp.spawnSync(cliPath,['version'],{encoding:'utf8',timeout:10000});
 if(ver.status!==0)throw Error('Arduino CLI not executable');
 console.log('BASELINE_SHA_PASS '+expected+'\n'+ver.stdout.trim());
 check('FULL_SETUP_LOOP_HOST', 'p0-01-full-loop-host.cjs',
  ['R001_R007_FULL_LOOP_CANDIDATE_HOST_CPP=','PASS; AVR_CANDIDATE_SEPARATE'],
  {...process.env,P001_RISK_CANDIDATE:'1'});
 check('PRIOR_SOURCE_EXTRACTED_HOST', 'p0-01-host-action-gate.cjs',
  ['SOURCE_EXTRACTED_BRANCH_HOST_CPP=42/42 PASS'],process.env);
 check('NEW_CANDIDATE_AVR_DUAL_BUILD', 'p0-01-r001-r007-candidate.cjs',
  ['RISK_CANDIDATE_AVR_COMPILE_PASS arduino:avr:pro:cpu=16MHzatmega328',
   'RISK_CANDIDATE_AVR_COMPILE_PASS arduino:avr:pro:cpu=8MHzatmega328',
   'RISK_CANDIDATE_DUAL_AVR_PASS=2/2'],{...process.env,ARDUINO_CLI:cliPath});
 console.log('\nP001_R001_R007_SOFTWARE_QA=PASS; HW_BENCH=NOT_RUN; RELEASE_BLOCKED');
} catch(e) {
 console.error('\nP001_R001_R007_SOFTWARE_QA=BLOCKED: '+e.message);
 console.error('FIRMWARE_UNCHANGED; HW_BENCH=NOT_RUN; RELEASE_BLOCKED');
 process.exitCode=2;
}
