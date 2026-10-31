#!/usr/bin/env node
'use strict';
// Software-only regression; mock time is NOT physical STOP latency.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os');
const crypto=require('node:crypto'),cp=require('node:child_process');
const dir=__dirname, root=path.resolve(dir,'..');
const fw=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
const mock=path.join(dir,'p0-02-stop-timing-native.cpp');
const expectedFirmware='fd7a78a0ec4aae33eb466dc1cdaae671fa3ca02f';
function sha(buf){return crypto.createHash('sha1').update('blob '+buf.length+'\0').update(buf).digest('hex');}
function readProtected(file,expected){
 const b=fs.readFileSync(file);const raw=sha(b);
 if(raw===expected)return b.toString('utf8');
 const normalized=Buffer.from(b.toString('utf8').replace(/\r\n/g,'\n'),'utf8');
 if(sha(normalized)===expected)return normalized.toString('utf8');
 throw Error('SOURCE_SHA_CHANGED '+path.basename(file));
}
// Match complete function bodies, not nearby comments or unrelated occurrences.
function body(source,signature){
 const p=source.indexOf(signature);
 if(p<0 || source.indexOf(signature,p+1)>=0)throw Error('MISSING_OR_DUPLICATE_FUNCTION '+signature);
 const open=source.indexOf('{',p+signature.length);
 if(open<0)throw Error('NO_BODY '+signature);
 let depth=0;
 for(let i=open;i<source.length;i++){
   if(source[i]==='{')depth++;
   if(source[i]==='}' && --depth===0)return source.slice(p,i+1).replace(/\r\n/g,'\n').trim();
   // Only count braces from the actual function body.
 }
 throw Error('UNBALANCED '+signature);
}
function run(bin,args){const x=cp.spawnSync(bin,args,{encoding:'utf8',timeout:60000});if(x.error||x.status!==0)throw Error('NATIVE_FAILED '+bin+' '+(x.error?.message||x.status)+' '+x.stderr+' '+x.stdout);return x.stdout;}
let temp;
try{
 const firmware=readProtected(fw,expectedFirmware);
 const cpp=fs.readFileSync(mock,'utf8');
 for(const signature of ['void usbApplyEvent(const perpompa_usb::Event &e)','boolean dose(long _steps, int _delay_us, long & inc)','void pump(int _delay_us)']){
   if(body(firmware,signature)!==body(cpp,signature))throw Error('MOCK_SOURCE_BODY_DRIFT '+signature);
 }
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-stop-native-'));
 const tests=[['BASE',cpp,true],['EEPROM_11_TO_10',cpp.replace('for(int i=0;i<11;i++)eepromWriteInt','for(int i=0;i<10;i++)eepromWriteInt'),false],['STEP_REMOVE_WAIT',cpp.replace('delayMicroseconds(_delay_us);\n  digitalWrite(MOTOR_STEP_PIN, LOW);','digitalWrite(MOTOR_STEP_PIN, LOW);'),false]];
 for(const [name,source,positive] of tests){
   if(source===cpp&&!positive)throw Error('MUTATION_NOT_APPLIED '+name);
   const src=path.join(temp,name+'.cpp');fs.writeFileSync(src,source);
   for(const hz of [16000000,8000000]){
     const exe=path.join(temp,name+'-'+hz+(process.platform==='win32'?'.exe':''));
     const args=['-std=c++11','-O0','-Wall','-Wextra','-pedantic','-DF_CPU='+hz+'UL',src,'-o',exe];
     run(process.env.CXX||'g++',args);
     const outcome=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000});
     if(positive){
       if(outcome.error||outcome.status!==0||!(outcome.stdout||'').includes('P002_STOP_TIMING_NATIVE_FCPU_'+hz+'=13/13 PASS'))throw Error('POSITIVE_FAILED '+hz);
       console.log('P002_STOP_NATIVE_'+hz+'=13/13 PASS; MOCK_ONLY');
     }else{
       if(outcome.error||outcome.status===0)throw Error('MUTANT_ESCAPED '+name+' '+hz);
       console.log('P002_STOP_MUTANT_'+name+'_'+hz+'=BLOCKED');
     }
   }
 }
 console.log('P002_STOP_SOURCE_BODY_GATE=PASS; PHYSICAL_STOP=NOT_RUN; FRAMING=REVIEW; RELEASE_BLOCKED');
}catch(e){console.error('P002_STOP_GATE=BLOCKED '+e.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
