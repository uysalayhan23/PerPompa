#!/usr/bin/env node
'use strict';
// Native tests for genuine SHA-locked production parser; tests expose framing risk, not physical STOP latency.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),crypto=require('node:crypto'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const parser=path.join(root,'p0-02-usb-parser.h');
const test=path.join(__dirname,'p0-02-parser-transitions-native.cpp');
const expectedParser='20527923030414d7d385b5102b9307446ce7f5c7';
function sha(b){return crypto.createHash('sha1').update('blob '+b.length+'\0').update(b).digest('hex');}
function protectedParser(){const raw=fs.readFileSync(parser);if(sha(raw)===expectedParser)return raw.toString('utf8');const s=Buffer.from(raw.toString('utf8').replace(/\r\n/g,'\n'));if(sha(s)===expectedParser)return s.toString('utf8');throw Error('PARSER_SHA_CHANGED');}
function invoke(command,args){const p=cp.spawnSync(command,args,{encoding:'utf8',timeout:60000});if(p.error)throw p.error;return p;}
let tmp;
try{
 const header=protectedParser();const cpp=fs.readFileSync(test,'utf8');
 tmp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-parser-transitions-'));
 for(const [kind,body,pass] of [
  ['BASE',header,true],
  ['MUTANT_QUIET41',header.replace('kQuietMs=40','kQuietMs=41'),false],
  ['MUTANT_STOP_DISABLED',header.replace("if(ch=='x')","if(ch=='X')"),false]
 ]){
  if(!pass&&body===header)throw Error('MUTANT_MISSING '+kind);
  const d=path.join(tmp,kind);fs.mkdirSync(d);fs.writeFileSync(path.join(d,'p0-02-usb-parser.h'),body);fs.writeFileSync(path.join(d,'native.cpp'),cpp);
  for(const hz of [16000000,8000000]){
    const exe=path.join(d,'native'+(process.platform==='win32'?'.exe':''));
    const c=invoke(process.env.CXX||'g++',['-std=c++11','-Wall','-Wextra','-Werror','-pedantic','-DF_CPU='+hz+'UL',path.join(d,'native.cpp'),'-o',exe]);
    if(c.status!==0)throw Error('COMPILER_FAILED '+kind+' '+hz+' '+c.stderr);
    const run=invoke(exe,[]);
    if(pass){if(run.status!==0||!run.stdout.includes('P002_TRANSITIONS_FCPU_'+hz+'=13/13 PASS'))throw Error('POSITIVE_FAILED '+hz+' '+run.stderr);console.log('P002_TRANSITIONS_FCPU_'+hz+'=13/13 PASS');}
    else {if(run.status===0)throw Error('MUTANT_ESCAPED '+kind+' '+hz);console.log('P002_'+kind+'_'+hz+'=BLOCKED');}
  }
 }
 console.log('P002_TRANSITIONS_GATE=PASS; FRAMING=REVIEW; PHYSICAL_STOP=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_TRANSITIONS_GATE=BLOCKED '+e.message);process.exitCode=2;}
finally{if(tmp)fs.rmSync(tmp,{recursive:true,force:true});}
