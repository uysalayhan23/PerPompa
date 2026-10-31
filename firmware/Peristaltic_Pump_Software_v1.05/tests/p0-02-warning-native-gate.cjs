#!/usr/bin/env node
'use strict';
// Targeted C++ proof of pointer-comparison defect and explicit no-op switch cases.
// Does not substitute full setup/loop and new dual AVR compile.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),cp=require('node:child_process');
const {build}=require('./p0-02-warning-candidate.cjs');
let temp;
try{
 const source=build();
 if((source.match(/strcmp\(menu\[1\]\.suffix,"uL"\)==0/g)||[]).length!==2||
    (source.match(/strcmp\(menu\[3\]\.suffix,"uL\/min"\)==0/g)||[]).length!==1)
    throw Error('WARNING_FIX_SOURCE_CHECK_FAILED');
 const src=path.join(__dirname,'p0-02-string-switch-native.cpp');
 if(!fs.existsSync(src))throw Error('CPP_TEST_MISSING');
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-p002-warnings-'));
 for(const fcpu of [16000000,8000000]){
  const exe=path.join(temp,'check-'+fcpu+(process.platform==='win32'?'.exe':''));
  const build=cp.spawnSync(process.env.CXX||'g++',['-std=c++17','-O0','-Wall','-Wextra','-Werror','-pedantic',
   '-fsanitize=undefined','-fno-sanitize-recover=all','-DF_CPU='+fcpu+'UL',src,'-o',exe],
   {encoding:'utf8',timeout:60000});
  if(build.stdout)process.stdout.write(build.stdout);
  if(build.stderr)process.stderr.write(build.stderr);
  if(build.error||build.status!==0)throw Error('NATIVE_COMPILE_FAIL '+fcpu+' '+(build.error?.message||build.status));
  const run=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000});
  if(run.stdout)process.stdout.write(run.stdout);
  if(run.stderr)process.stderr.write(run.stderr);
  if(run.error||run.status!==0||!(run.stdout||'').includes('P002_STRING_SWITCH_TARGET_CPP=7/7 PASS'))
   throw Error('NATIVE_TEST_FAIL '+fcpu+' '+(run.error?.message||run.status));
 }
 console.log('P002_WARNING_TARGET_NATIVE_TWO_CPU=2/2 PASS; FULL_SOURCE=NOT_RUN; AVR=NOT_RUN; ACTIVE_INO_UNCHANGED');
}catch(e){console.error('P002_WARNING_TARGET_NATIVE=BLOCKED '+e.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
