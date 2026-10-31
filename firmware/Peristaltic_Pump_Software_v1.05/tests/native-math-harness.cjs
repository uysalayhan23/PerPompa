#!/usr/bin/env node
'use strict';
// Compiles unmodified C++ function bodies extracted from the real .ino.
// HOST_CPP only: NOT full firmware, not AVR, not a hardware/flow test.
// Requires g++ and Node 18+. Uses OS temp directory, never edits repository.
const fs=require('node:fs'),os=require('node:os'),path=require('node:path');
const cp=require('node:child_process'),assert=require('node:assert/strict');
const sourcePath=path.resolve(__dirname,'../Peristaltic_Pump_Software_v1.05.ino');
const text=fs.readFileSync(sourcePath,'utf8');
function extract(name){
 const pat=new RegExp('\\blong\\s+'+name+'\\s*\\(');
 const m=pat.exec(text);if(!m)throw new Error('Function missing: '+name);
 let b=text.indexOf('{',m.index);if(b<0)throw new Error('Opening brace missing');
 let nesting=0;for(let i=b;i<text.length;i++){if(text[i]==='{')nesting++;if(text[i]==='}'&&!--nesting)return text.slice(m.index,i+1);}
 throw new Error('Closing brace missing');
}
function def(name){
 const line=text.split(/\r?\n/).find(s=>new RegExp('^#define\\s+'+name+'\\s+').test(s));
 if(!line)throw new Error('Required define absent: '+name);return line;
}
const defs=['STEP_MODE','STEPS_PER_FULL_ROT','CALIBR_ROTATIONS','CALIBR_DECIMALS','MICROSEC_PER_SEC'].map(def).join('\n');
const originalFunctions=extract('steps_calc')+'\n'+extract('delay_us_calc');
const cxx=[
 '#include <cmath>','#include <climits>','#include <cstdint>','#include <iostream>','#include <string>',
 defs,'const int CALIBR_DECIMAL_CORR = pow(10,CALIBR_DECIMALS);',originalFunctions,
 'int main(int argc,char**argv){',
 '  std::string mode=argc>1?argv[1]:"positive";',
 '  if(mode=="positive"){',
 '    if(steps_calc(10,0,5000,1)!=4800) return 11;',
 '    if(delay_us_calc(100,0,5000,1)!=625) return 12;',
 '    if(steps_calc(3,2,0,0)!=2400) return 13;',
 '    if(delay_us_calc(10,2,0,0)!=3750) return 14;',
 '    std::cout<<"HOST_CPP_POSITIVE_PASS (4 cases)\\n";return 0;',
 '  }',
 '  if(mode=="zero_cal"){volatile long v=steps_calc(10,0,0,1);std::cout<<v<<"\\n";return 0;}',
 '  if(mode=="zero_rate"){volatile long v=delay_us_calc(0,0,5000,1);std::cout<<v<<"\\n";return 0;}',
 '  if(mode=="overflow32"){',
 '    double expected=24000.0*9999.0*1000.0/(1.0*10.0);',
 '    if(!(expected>INT32_MAX))return 15;',
 '    std::cout<<"MODEL_BOUNDARY_REVIEW expected_steps="<<expected<<" INT32_MAX="<<INT32_MAX<<"\\n";return 0;',
 '  }',
 '  return 98;',
 '}'
].join('\n');
const tmp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-hostcpp-'));
const src=path.join(tmp,'math.cpp'),exe=path.join(tmp,'math-test');
const run=(tool,args)=>cp.spawnSync(tool,args,{encoding:'utf8',timeout:30000,maxBuffer:1024*1024});
try{
 fs.writeFileSync(src,cxx);
 const cc=run(process.env.CXX||'g++',['-std=c++17','-O0','-g','-fsanitize=undefined,float-cast-overflow','-fno-sanitize-recover=all',src,'-o',exe]);
 if(cc.error||cc.status!==0){
  console.error('HOST_CPP=NOT_RUN: compiler unavailable or compile error:',cc.error?.message||cc.stderr);
  process.exitCode=2;
 }else{
  let x=run(exe,['positive']);
  if(x.error||x.status!==0){console.error('HOST_CPP_POSITIVE_FAIL',x.stderr,x.stdout);process.exitCode=1;}
  else console.log(x.stdout.trim());
  for(const kind of ['zero_cal','zero_rate']){
   x=run(exe,[kind]);
   if(x.status!==0&&/runtime error:|float-cast-overflow/i.test(x.stderr)){
    console.log('KNOWN_DEFECT_HOST_CPP_REPRODUCED',kind,'sanitizer='+x.stderr.trim().split('\n')[0]);
   }else{
    console.error('NEGATIVE_NOT_CONFIRMED',kind,'status='+x.status,'stdout='+x.stdout.trim(),'stderr='+x.stderr.trim());process.exitCode=1;
   }
  }
  x=run(exe,['overflow32']);
  if(x.status===0) console.log(x.stdout.trim()); else {console.error('MODEL_BOUNDARY_CHECK_FAIL',x.stderr);process.exitCode=1;}
  console.log('AVR_COMPILE=NOT_RUN HW_BENCH=NOT_RUN HW_PUMP=NOT_RUN RELEASE_BLOCKED');
 }
}finally{fs.rmSync(tmp,{recursive:true,force:true});}
