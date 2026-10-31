#!/usr/bin/env node
'use strict';
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..');const header=path.join(root,'p0-02-usb-parser.h');
const cpp=path.join(__dirname,'p0-02-usb-parser-native.cpp');let temp;
try{
 if(!fs.existsSync(header)||!fs.existsSync(cpp))throw Error('P002_INPUT_MISSING');
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-usb-p002-'));
 for(const clock of [16000000,8000000]){
  const exe=path.join(temp,'check-'+clock+(process.platform==='win32'?'.exe':''));
  const args=['-std=c++11','-O0','-Wall','-Wextra','-Werror','-pedantic',
    '-fsanitize=undefined','-fno-sanitize-recover=all','-DF_CPU='+clock+'UL',
    '-I',root,cpp,'-o',exe];
  const build=cp.spawnSync(process.env.CXX||'g++',args,{encoding:'utf8',timeout:60000,maxBuffer:2*1024*1024});
  if(build.stdout)process.stdout.write(build.stdout);
  if(build.stderr)process.stderr.write(build.stderr);
  if(build.error||build.status!==0)throw Error('CPP_BUILD_'+clock+' '+(build.error?.message||build.status));
  const run=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000,maxBuffer:2*1024*1024});
  if(run.stdout)process.stdout.write(run.stdout);
  if(run.stderr)process.stderr.write(run.stderr);
  const marker='P002_USB_NONBLOCK_NATIVE_FCPU_'+clock+'=36/36 PASS';
  if(run.error||run.status!==0||!(run.stdout||'').includes(marker))throw Error('CPP_REGRESSION_'+clock+' '+(run.error?.message||run.status));
 }
 console.log('P002_USB_NONBLOCK_NATIVE_TWO_FCPU_PASS=2/2; FULL_SKETCH=NOT_RUN; AVR=NOT_RUN; HW=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_USB_NONBLOCK_NATIVE=BLOCKED '+e.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
