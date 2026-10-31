#!/usr/bin/env node
'use strict';
const fs=require('node:fs'), path=require('node:path'), os=require('node:os'), cp=require('node:child_process');
const home=path.resolve(__dirname,'..');
const source=path.join(__dirname,'hw-setup-profile-native.cpp');
const header=path.join(home,'hw-setup-profile.h');
let workspace;
try{
 if(!fs.existsSync(source)||!fs.existsSync(header))throw Error('HW_SETUP_SOURCE_MISSING');
 workspace=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-hw-profile-'));
 for(const hz of [16000000,8000000]){
  const exe=path.join(workspace,'profile-'+hz+(process.platform==='win32'?'.exe':''));
  const args=['-std=c++11','-Wall','-Wextra','-Werror','-pedantic','-O0',
   '-fsanitize=undefined','-fno-sanitize-recover=all',
   '-DF_CPU='+hz+'UL','-I',home,source,'-o',exe];
  const r=cp.spawnSync(process.env.CXX||'g++',args,{encoding:'utf8',timeout:60000,maxBuffer:1024*1024});
  if(r.stdout)process.stdout.write(r.stdout);
  if(r.stderr)process.stderr.write(r.stderr);
  if(r.error||r.status!==0)throw Error('HOST_CPP_COMPILE_FAIL '+hz+': '+(r.error?.message||r.status));
  const run=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000,maxBuffer:1024*1024});
  if(run.stdout)process.stdout.write(run.stdout);
  if(run.stderr)process.stderr.write(run.stderr);
  const success='HW_SETUP_NATIVE_FCPU_'+hz+'=';
  if(run.error||run.status!==0||!run.stdout?.includes(success)||!run.stdout?.includes('PASS; INO_INTEGRATION=NOT_RUN'))
   throw Error('HOST_CPP_NEGATIVE_POSITIVE_FAIL '+hz+': '+(run.error?.message||run.status));
 }
 console.log('HW_SETUP_NATIVE_TWO_CPU_PASS=2/2; AVR_FIRMWARE=NOT_RUN; HW_BENCH=NOT_RUN; RELEASE_BLOCKED');
}catch(err){console.error('HW_SETUP_NATIVE=BLOCKED '+err.message);process.exitCode=2;}
finally{if(workspace)fs.rmSync(workspace,{recursive:true,force:true});}
