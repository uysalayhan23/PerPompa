#!/usr/bin/env node
'use strict';
// Only the accepted runtime header hot-path; does NOT certify whole sketch, hardware or release.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..'),input=path.join(__dirname,'hw-setup-01d-runtime-hot.cpp');
let temp;
try{
 if(!fs.existsSync(input)||!fs.existsSync(path.join(root,'hw-setup-profile.h'))||
    !fs.existsSync(path.join(root,'hw-setup-runtime.h')))throw Error('HW01D_INPUT_MISSING');
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-hot01d-'));
 for(const cpu of [16000000,8000000]){
  const binary=path.join(temp,'probe-'+cpu+(process.platform==='win32'?'.exe':''));
  const args=['-std=c++11','-O0','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=undefined',
    '-fno-sanitize-recover=all','-DF_CPU='+cpu+'UL','-I',root,input,'-o',binary];
  const build=cp.spawnSync(process.env.CXX||'g++',args,{encoding:'utf8',timeout:60000,maxBuffer:2097152});
  if(build.stdout)process.stdout.write(build.stdout);
  if(build.stderr)process.stderr.write(build.stderr);
  if(build.error||build.status!==0)throw Error('HOT_CXX_COMPILE_'+cpu+' '+(build.error?.message||'exit '+build.status));
  const run=cp.spawnSync(binary,[],{encoding:'utf8',timeout:60000,maxBuffer:2097152});
  if(run.stdout)process.stdout.write(run.stdout);
  if(run.stderr)process.stderr.write(run.stderr);
  const marker='RUNTIME_HOT_PATH_CPU_'+cpu+'=12/12 PASS';
  if(run.error||run.status!==0||!(run.stdout||'').includes(marker))throw Error('HOT_CXX_RUN_'+cpu+' '+(run.error?.message||'exit '+run.status));
 }
 console.log('HW_SETUP_01D_RUNTIME_HOT_TWO_CPU_PASS=2/2; FULL_SKETCH=NOT_RUN; AVR=NOT_RUN; HW=NOT_RUN');
}catch(e){console.error('HW_SETUP_01D_RUNTIME_HOT=BLOCKED '+e.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
