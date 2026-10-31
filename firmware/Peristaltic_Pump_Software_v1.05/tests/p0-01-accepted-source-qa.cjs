#!/usr/bin/env node
'use strict';
// Post-merge P0-01 software regression: test accepted v1.05 source, not re-patch it.
// No hardware flash or release authorization. Fail closed on any mismatch or test failure.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),
      crypto=require('node:crypto'),cp=require('node:child_process');
const base=path.resolve(__dirname,'..'),repo=path.resolve(__dirname,'../../..');
const active=path.join(base,'Peristaltic_Pump_Software_v1.05.ino');
const archive=path.join(repo,'firmware/archive/Peristaltic_Pump_Software_v1.04/Peristaltic_Pump_Software_v1.04.ino');
const accepted='6617cc38bde9bddf1c32cd0506915c0054556127';
const legacy='0d965c08947ba0b3170d22fbff806fc2e6f1e83a';
function sha(file){const bytes=fs.readFileSync(file);return crypto.createHash('sha1').update('blob '+bytes.length+'\0').update(bytes).digest('hex');}
function run(label,command,args,options,expected){
 const result=cp.spawnSync(command,args,{encoding:'utf8',timeout:180000,maxBuffer:8*1024*1024,...options});
 if(result.stdout)process.stdout.write(result.stdout);
 if(result.stderr)process.stderr.write(result.stderr);
 const out=(result.stdout||'')+'\n'+(result.stderr||'');
 if(result.error || result.status!==0 || !out.includes(expected))
  throw Error(label+' FAILED: '+(result.error?.message||'exit '+result.status)+' expected '+expected);
 console.log(label+'=PASS');
}
let tmp=null;
try {
 if(!fs.existsSync(active)||!fs.existsSync(archive)||sha(active)!==accepted||sha(archive)!==legacy)
  throw Error('ACTIVE_OR_ARCHIVE_GIT_BLOB_MISMATCH');
 console.log('ACCEPTED_SOURCE_SHA_PASS '+accepted);
 const fixture=fs.readFileSync(path.join(__dirname,'p0-01-full-loop-host.cjs'),'utf8');
 const stubMatch=fixture.match(/^const stub=(.*);$/m);
 const testsMatch=fixture.match(/^if\(riskMode\)tests=(.*);$/m);
 if(!stubMatch||!testsMatch)throw Error('CXX_TEST_FIXTURE_MISSING');
 const stub=JSON.parse(stubMatch[1]),tests=JSON.parse(testsMatch[1]);
 if(!tests.includes('R001_R007_FULL_LOOP_CANDIDATE_HOST_CPP=%d/%d PASS'))throw Error('CXX_TEST_SIGNATURE_MISMATCH');
 const source=fs.readFileSync(active,'utf8');
 const proto=[...source.matchAll(/^(void|boolean|int8_t|long|int)\s+([a-zA-Z_]\w*)\s*\(([^)]*)\)\s*\{/gm)]
    .map(m=>m[1]+' '+m[2]+'('+m[3]+');').join('\n');
 const sketch=source.replace(/^[ \t]*#include[^\r\n]*/gm,'');
 tmp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-p001-accepted-'));
 const unit=path.join(tmp,'sketch.cpp');
 const binary=path.join(tmp,process.platform==='win32'?'sketch.exe':'sketch');
 fs.writeFileSync(unit,stub+'\n'+proto+'\n'+sketch+'\n'+tests);
 const cxx=process.env.CXX||'g++';
 run('ACCEPTED_FULL_SETUP_LOOP_HOST',cxx,
  ['-std=c++17','-O0','-Wall','-Wextra','-Wno-unused-parameter',
   '-fsanitize=undefined,float-cast-overflow','-fno-sanitize-recover=all',unit,'-o',binary],
  {timeout:60000},''); // compiler must succeed, no marker
 run('ACCEPTED_FULL_LOOP_32',binary,[],{timeout:60000},
     'R001_R007_FULL_LOOP_CANDIDATE_HOST_CPP=32/32 PASS');
 // Preserve the established 42-test negative/positive legacy regression from
 // immutable v1.04 + the original patch, without attempting to patch accepted main twice.
 const legacyTests=path.join(tmp,'firmware/Peristaltic_Pump_Software_v1.05/tests');
 const legacyArchive=path.join(tmp,'firmware/archive/Peristaltic_Pump_Software_v1.04');
 fs.mkdirSync(legacyTests,{recursive:true});fs.mkdirSync(legacyArchive,{recursive:true});
 fs.copyFileSync(archive,path.join(tmp,'firmware/Peristaltic_Pump_Software_v1.05/Peristaltic_Pump_Software_v1.05.ino'));
 fs.copyFileSync(archive,path.join(legacyArchive,'Peristaltic_Pump_Software_v1.04.ino'));
 for(const name of ['p0-01-candidate-dual-build.cjs','p0-01-host-action-gate.cjs'])
  fs.copyFileSync(path.join(__dirname,name),path.join(legacyTests,name));
 run('HISTORICAL_SOURCE_EXTRACTED_HOST_42',process.execPath,
  [path.join(legacyTests,'p0-01-host-action-gate.cjs')],{timeout:60000},
  'SOURCE_EXTRACTED_BRANCH_HOST_CPP=42/42 PASS');
 // Native Arduino compilation uses the accepted .ino itself, not a generated candidate.
 const cli=process.env.ARDUINO_CLI||'arduino-cli';
 for(const cpu of ['16MHzatmega328','8MHzatmega328']){
  const fqbn='arduino:avr:pro:cpu='+cpu;
  const result=cp.spawnSync(cli,['compile','--fqbn',fqbn,'--build-path',path.join(tmp,'build-'+cpu),base],
    {encoding:'utf8',timeout:180000,maxBuffer:8*1024*1024});
  if(result.stdout)process.stdout.write(result.stdout);
  if(result.stderr)process.stderr.write(result.stderr);
  if(result.error||result.status!==0)throw Error('ACCEPTED_AVR_BUILD_FAILED '+fqbn+': '+(result.error?.message||'exit '+result.status));
  console.log('ACCEPTED_AVR_COMPILE_PASS '+fqbn);
 }
 console.log('P001_ACCEPTED_SOFTWARE_QA=PASS; HW_BENCH=NOT_RUN; HW_PUMP=NOT_RUN; RELEASE_BLOCKED');
} catch(e){
 console.error('P001_ACCEPTED_SOFTWARE_QA=BLOCKED: '+e.message);
 console.error('HW_BENCH=NOT_RUN; HW_PUMP=NOT_RUN; RELEASE_BLOCKED');
 process.exitCode=2;
} finally {if(tmp)fs.rmSync(tmp,{recursive:true,force:true});}
