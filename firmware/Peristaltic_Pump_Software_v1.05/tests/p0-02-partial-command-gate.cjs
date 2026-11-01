#!/usr/bin/env node
'use strict';
const fs=require('fs'),path=require('path'),os=require('os'),crypto=require('crypto'),cp=require('child_process');
const root=path.resolve(__dirname,'..');
const header=path.join(root,'p0-02-usb-parser.h');
const test=path.join(__dirname,'p0-02-partial-command-native.cpp');
const expected='20527923030414d7d385b5102b9307446ce7f5c7';
const gitSha=b=>crypto.createHash('sha1').update('blob '+b.length+'\0').update(b).digest('hex');
let temp;
try {
 const raw=fs.readFileSync(header);const normal=Buffer.from(raw.toString('utf8').replace(/\r\n/g,'\n'));
 if(gitSha(raw)!==expected && gitSha(normal)!==expected)throw Error('PARSER_SHA_MISMATCH');
 const source=normal.toString('utf8'),testCode=fs.readFileSync(test,'utf8');
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'p002-partial-'));
 for(const [label,headerText,pass] of [
  ['REAL',source,true],
  ['QUIET_41',source.replace('kQuietMs=40','kQuietMs=41'),false],
  ['STOP_DISABLED',source.replace("if(ch=='x')","if(ch=='X')"),false]]) {
  if(!pass&&headerText===source)throw Error('MUTATION_ANCHOR_MISSING '+label);
  const dir=path.join(temp,label);fs.mkdirSync(dir);fs.writeFileSync(path.join(dir,'p0-02-usb-parser.h'),headerText);fs.writeFileSync(path.join(dir,'test.cpp'),testCode);
  for(const hz of [16000000,8000000]){
   const exe=path.join(dir,'test'+(process.platform==='win32'?'.exe':''));
   const c=cp.spawnSync(process.env.CXX||'g++',['-std=c++11','-Wall','-Wextra','-Werror','-pedantic','-DF_CPU='+hz+'UL',path.join(dir,'test.cpp'),'-o',exe],{encoding:'utf8'});
   if(c.error||c.status!==0)throw Error('COMPILE_FAILED '+label+' '+hz+' '+String(c.error||c.stderr));
   const run=cp.spawnSync(exe,[],{encoding:'utf8'});
   if(run.error)throw run.error;
   if(pass){if(run.status!==0||!run.stdout.includes('P002_PARTIAL_FCPU_'+hz+'=15/15 PASS'))throw Error('BASE_FAILED '+hz+' '+run.stderr);console.log('P002_PARTIAL_'+hz+'=15/15 PASS');}
   else {if(run.status===0)throw Error('MUTANT_ESCAPED '+label+' '+hz);console.log('P002_'+label+'_'+hz+'=BLOCKED');}
  }
 }
 console.log('P002_PARTIAL_GATE=PASS; PARSER_ONLY; FRAMING=REVIEW; PHYSICAL_STOP=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_PARTIAL_GATE=BLOCKED '+e.message);process.exitCode=2;}
finally {if(temp)fs.rmSync(temp,{recursive:true,force:true});}
