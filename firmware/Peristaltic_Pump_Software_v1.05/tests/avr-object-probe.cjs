#!/usr/bin/env node
'use strict';
// AVR-target object probe of two unchanged source calculator functions.
// NOT the Arduino builder, full firmware link, or a hardware test.
const fs=require('node:fs'),os=require('node:os'),path=require('node:path');
const cp=require('node:child_process');
const sketch=process.argv[2] || path.join(__dirname,'../Peristaltic_Pump_Software_v1.05.ino');
const source=fs.readFileSync(sketch,'utf8');
function extract(name){
 const re=new RegExp('\\blong\\s+'+name+'\\s*\\(');
 const m=re.exec(source);if(!m)throw Error('MISSING '+name);
 const start=source.indexOf('{',m.index);let d=0;
 for(let i=start;i<source.length;i++){if(source[i]==='{')d++;if(source[i]==='}'&&!--d)return source.slice(m.index,i+1)}
 throw Error('UNBALANCED '+name);
}
function define(n){
 const m=source.match(new RegExp('^#define\\s+'+n+'\\s+[^\\r\\n]+','m'));
 if(!m)throw Error('MISSING DEFINE '+n);return m[0];
}
const needed=['STEP_MODE','STEPS_PER_FULL_ROT','CALIBR_ROTATIONS','CALIBR_DECIMALS','MICROSEC_PER_SEC'];
const code=[
 'extern "C" double pow(double,double);',
 ...needed.map(define),
 'const int CALIBR_DECIMAL_CORR = pow(10,CALIBR_DECIMALS);',
 extract('steps_calc'),extract('delay_us_calc')
].join('\n');
const tmp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-avr-object-'));
try{
 const file=path.join(tmp,'selected.cpp');fs.writeFileSync(file,code);
 for(const mhz of [8,16]){
  const out=path.join(tmp,'avr'+mhz+'.o');
  const args=['--target=avr','-mmcu=atmega328p','-DF_CPU='+mhz+'000000UL','-std=c++11','-Wall','-Wextra','-c',file,'-o',out];
  const r=cp.spawnSync(process.env.AVR_CLANG||'clang++',args,{encoding:'utf8',timeout:30000});
  if(r.error){console.error('AVR_OBJECT_NOT_RUN',r.error.message);process.exit(2)}
  if(r.status!==0){console.error('AVR_OBJECT_FAIL',mhz,r.stderr);process.exit(1)}
  const data=fs.readFileSync(out);
  if(data.length<20 || data.toString('ascii',0,4)!=='\x7fELF' || data.readUInt16LE(18)!==83){
    console.error('INVALID_AVR_ELF');process.exit(1);
  }
  const hits=[...r.stderr.matchAll(/warning:/g)].length;
  console.log('AVR_OBJECT_PASS F_CPU='+mhz+'MHz warnings='+hits);
  if(r.stderr)console.log('AVR_WARNINGS_'+mhz+' '+r.stderr.split('\n').filter(x=>x.includes('warning:')).join(' | '));
 }
 console.log('AVR_OBJECT=PASS; AVR_ARDUINO_BUILD=NOT_RUN; FULL_FIRMWARE_LINK=NOT_RUN; HW=NOT_RUN');
}finally{fs.rmSync(tmp,{recursive:true,force:true})}
