#!/usr/bin/env node
'use strict';
// Reproduces a framing ambiguity with actual accepted USB parser header.
// Positive native test result means the RISK EXISTS, not that STOP timing is safe.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),
  crypto=require('node:crypto'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const cpp=path.join(__dirname,'p0-02-framing-review-native.cpp');
const header=path.join(root,'p0-02-usb-parser.h');
const firmware=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
const expected={
 [cpp]:'201ed2993460126d3054f6f79349a4366e329dee',
 [header]:'20527923030414d7d385b5102b9307446ce7f5c7',
 [firmware]:'5b3c5a29ce8ce5eee0941469738b7e088d60a1bb'
};
function blob(file){const b=fs.readFileSync(file);return crypto.createHash('sha1')
  .update('blob '+b.length).update(Buffer.from([0])).update(b).digest('hex');}
let dir;
try{
 for(const [file,sha] of Object.entries(expected))if(blob(file)!==sha)
   throw Error('SOURCE_SHA_CHANGED '+path.basename(file));
 dir=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-p002-framing-'));
 for(const hz of [16000000,8000000]){
   const exe=path.join(dir,'native-'+hz+(process.platform==='win32'?'.exe':''));
   const args=['-std=c++11','-O0','-Wall','-Wextra','-Werror','-pedantic',
    '-fsanitize=undefined','-fno-sanitize-recover=all','-DF_CPU='+hz+'UL',
    '-I',root,cpp,'-o',exe];
   const c=cp.spawnSync(process.env.CXX||'g++',args,{encoding:'utf8',timeout:60000});
   if(c.stdout)process.stdout.write(c.stdout);
   if(c.stderr)process.stderr.write(c.stderr);
   if(c.error||c.status!==0)throw Error('COMPILER_'+hz+' '+(c.error?.message||c.status));
   const r=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000});
   if(r.stdout)process.stdout.write(r.stdout);
   if(r.stderr)process.stderr.write(r.stderr);
   if(r.error||r.status!==0||
     !(r.stdout||'').includes('P002_FRAMING_SOURCE_FCPU_'+hz+'=23/23 PASS; TRUNCATED_VALID_LAST_FIELD_RISK=REPRODUCED'))
     throw Error('CPP_REGRESSION_'+hz+' '+(r.error?.message||r.status));
 }
 console.log('P002_FRAMING_NEGATIVE_REPRO=2/2 PASS; FRAMING_SAFETY=REVIEW; HW_STOP=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_FRAMING_REVIEW=BLOCKED '+e.message);process.exitCode=2;}
finally{if(dir)fs.rmSync(dir,{recursive:true,force:true});}
