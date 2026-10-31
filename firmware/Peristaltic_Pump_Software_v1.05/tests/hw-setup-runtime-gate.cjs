#!/usr/bin/env node
'use strict';
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..'),input=path.join(__dirname,'hw-setup-runtime-native.cpp');
const h=path.join(root,'hw-setup-runtime.h');let temp;
try{
 if(!fs.existsSync(input)||!fs.existsSync(h)||!fs.existsSync(path.join(root,'hw-setup-profile.h')))throw Error('INPUTS_MISSING');
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-hw-runtime-'));
 for(const fcpu of [16000000,8000000]){
   const exe=path.join(temp,'runtime-'+fcpu+(process.platform==='win32'?'.exe':''));
   const args=['-std=c++11','-Wall','-Wextra','-Werror','-pedantic','-O0','-fsanitize=undefined','-fno-sanitize-recover=all','-DF_CPU='+fcpu+'UL','-I',root,input,'-o',exe];
   const r=cp.spawnSync(process.env.CXX||'g++',args,{encoding:'utf8',timeout:60000,maxBuffer:2*1024*1024});
   if(r.stdout)process.stdout.write(r.stdout);if(r.stderr)process.stderr.write(r.stderr);
   if(r.error||r.status!==0)throw Error('COMPILE_'+fcpu+': '+(r.error?.message||r.status));
   const run=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000,maxBuffer:2*1024*1024});
   if(run.stdout)process.stdout.write(run.stdout);if(run.stderr)process.stderr.write(run.stderr);
   if(run.error||run.status!==0||!(run.stdout||'').includes('HW_SETUP_RUNTIME_FCPU_'+fcpu+'=486/486 PASS'))throw Error('REGRESSION_'+fcpu+': '+(run.error?.message||run.status));
 }
 console.log('HW_SETUP_RUNTIME_CPP_TWO_FCPU_PASS=2/2; ACTIVE_INO=NOT_MODIFIED; AVR=NOT_RUN; RELEASE_BLOCKED');
}catch(err){console.error('HW_SETUP_RUNTIME_CPP=BLOCKED '+err.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
