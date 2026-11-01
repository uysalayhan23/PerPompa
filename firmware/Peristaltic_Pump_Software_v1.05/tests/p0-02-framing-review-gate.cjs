#!/usr/bin/env node
'use strict';
// Current V2 framing regression. Legacy 40ms auto-complete reproduction is historical,
// not a valid current parser acceptance test. HOST_ONLY, never hardware approval.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),
  crypto=require('node:crypto'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const inputs=[
  ['p0-02-framing-review-native.cpp',path.join(__dirname,'p0-02-framing-review-native.cpp'),'c19080b6544d712f5393157a4e5f5a79d5b33d45'],
  ['p0-02-usb-parser.h',path.join(root,'p0-02-usb-parser.h'),'c8d818ca461f7cbfb3cf45ccf06cf2f589d6d604'],
  ['v2-parser.h',path.join(root,'v2-parser.h'),'1933e7f86d9fc8a4aff4d6c2ac6e2e5714ceafe8'],
  ['Peristaltic_Pump_Software_v1.05.ino',path.join(root,'Peristaltic_Pump_Software_v1.05.ino'),'fd7a78a0ec4aae33eb466dc1cdaae671fa3ca02f'],
];
function blob(bytes){return crypto.createHash('sha1').update(Buffer.from('blob '+bytes.length+'\0')).update(bytes).digest('hex')}
function verify([name,file,expected]){
 if(!fs.existsSync(file))throw Error('SOURCE_NOT_FOUND '+name);
 const raw=fs.readFileSync(file);
 // Windows CRLF checkout is accepted only if normalizing the line endings reproduces the exact Git blob.
 if(blob(raw)===expected)return;
 const lf=Buffer.from(raw.toString('utf8').replace(/\r\n/g,'\n'));
 if(blob(lf)!==expected)throw Error('SOURCE_SHA_CHANGED '+name);
}
function selectCompiler(){
 const c=process.env.CXX||'g++';
 let r=cp.spawnSync(c,['--version'],{encoding:'utf8',timeout:10000});
 if(!r.error&&r.status===0)return c;
 if(process.platform==='win32'&&!process.env.CXX){
   const alt='C:\\msys64\\clang64\\bin\\clang++.exe';
   r=cp.spawnSync(alt,['--version'],{encoding:'utf8',timeout:10000});
   if(!r.error&&r.status===0)return alt;
 }
 throw Error('CXX_NOT_FOUND '+c);
}
let dir;
try{
 inputs.forEach(verify);
 console.log('P002_V2_FRAMING_SOURCE_SHA=PASS (native + adapter + CRC parser + original firmware)');
 const tool=selectCompiler();
 dir=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-p002-v2-framing-'));
 let passed=0;
 for(const hz of [8000000,16000000]){
   const exe=path.join(dir,'native-'+hz+(process.platform==='win32'?'.exe':''));
   const args=['-std=c++11','-O0','-Wall','-Wextra','-Werror','-pedantic',
     '-DF_CPU='+hz+'UL','-I',root,inputs[0][1],'-o',exe];
   const opt={encoding:'utf8',timeout:60000,maxBuffer:1024*1024};
   const c=cp.spawnSync(tool,args,opt);
   if(c.error||c.status!==0)throw Error('COMPILE_'+hz+' '+(c.error?.message||c.stderr||c.status));
   const runtimeEnv={...process.env};
   if(process.platform==='win32'&&/[\\/]/.test(tool))runtimeEnv.PATH=path.dirname(tool)+path.delimiter+(process.env.PATH||'');
   const result=cp.spawnSync(exe,[],{...opt,env:runtimeEnv});
   if(result.stdout)process.stdout.write(result.stdout);
   if(result.stderr)process.stderr.write(result.stderr);
   if(result.error||result.status!==0||!result.stdout?.includes('P002_V2_FRAMING_FCPU_'+hz+'=31/31 PASS'))
     throw Error('FRAMING_TEST_'+hz+' exit='+result.status+' '+(result.error?.message||''));
   passed++;
 }
 if(passed!==2)throw Error('INCOMPLETE_'+passed+'/2');
 console.log('P002_V2_FRAMING_GATE=PASS 2/2 HOST_ONLY; SOFTWARE_SECURITY_REVIEW; PHYSICAL_NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_V2_FRAMING_GATE=BLOCKED '+e.message);process.exitCode=2;}
finally{if(dir)fs.rmSync(dir,{recursive:true,force:true});}
