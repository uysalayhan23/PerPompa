#!/usr/bin/env node
'use strict';
// Source-tied native side-effect gate; no physical timing or release certification.
const fs=require('fs'),path=require('path'),os=require('os'),crypto=require('crypto'),cp=require('child_process');
const firmware=process.argv[2],parser=process.argv[3],template=path.join(__dirname,'combined-template.cpp');
const sha=b=>crypto.createHash('sha1').update('blob '+b.length+'\0').update(b).digest('hex');
function checked(file,want){const b=fs.readFileSync(file);if(sha(b)===want)return b.toString('utf8');const n=Buffer.from(b.toString('utf8').replace(/\r\n/g,'\n'));if(sha(n)===want)return n.toString('utf8');throw Error('GIT_BLOB_SHA_MISMATCH '+path.basename(file));}
function block(s,startPattern,anchor){const found=[...s.matchAll(startPattern)];if(found.length!==1)throw Error('ANCHOR_COUNT '+anchor+' '+found.length);let i=s.indexOf('{',found[0].index);if(i<0)throw Error('NO_BRACE '+anchor);let depth=0;for(let j=i;j<s.length;j++){if(s[j]==='{')depth++;else if(s[j]==='}'&&--depth===0)return s.slice(found[0].index,j+1);}throw Error('UNCLOSED '+anchor);}
function trial(body,app,header,label,pass){const tmp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-combined-'));try{
 fs.writeFileSync(path.join(tmp,'p0-02-usb-parser.h'),header);
 const tpl=fs.readFileSync(template,'utf8');if(!tpl.includes('/*APPLY_INSERT*/')||!tpl.includes('/*CASE9_INSERT*/'))throw Error('TEMPLATE_ANCHORS');
 const cpp=tpl.replace('/*APPLY_INSERT*/',app).replace('/*CASE9_INSERT*/',body);
 const file=path.join(tmp,'test.cpp');fs.writeFileSync(file,cpp);
 for(const hz of [16000000,8000000]){
  const exe=path.join(tmp,'test-'+hz+(process.platform==='win32'?'.exe':''));
  const flags=['-std=c++11','-Wall','-Wextra','-O0','-DF_CPU='+hz,file,'-o',exe];
  const c=cp.spawnSync(process.env.CXX||'g++',flags,{encoding:'utf8',timeout:60000});if(c.error||c.status!==0)throw Error('COMPILE '+label+' '+hz+' '+String(c.error||c.stderr));
  const x=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000});const ok=!x.error&&x.status===0&&x.stdout.includes('P002_COMBINED_FCPU_'+hz+'=16/16 PASS');
  if(ok!==pass)throw Error('RESULT '+label+' '+hz+' '+JSON.stringify({exit:x.status,stdout:x.stdout,stderr:x.stderr}));
  console.log('P002_COMBINED_'+label+'_'+hz+'='+(pass?'16/16 PASS':'BLOCKED'));
 }
}finally{fs.rmSync(tmp,{recursive:true,force:true});}}
try{
 if(!firmware||!parser)throw Error('USAGE: node combined-gate.cjs <firmware.ino> <parser.h>');
 const fw=checked(firmware,'fd7a78a0ec4aae33eb466dc1cdaae671fa3ca02f');
 const header=checked(parser,'20527923030414d7d385b5102b9307446ce7f5c7');
 const body=block(fw,/^\s*case 9: \{/gm,'case9');
 const app=block(fw,/^void usbApplyEvent\(const perpompa_usb::Event &e\)\s*\{/gm,'usbApplyEvent');
 if(!body.includes('usbParser.tick')||!app.includes('eepromWriteInt'))throw Error('SOURCE_INVARIANTS');
 trial(body,app,header,'REAL',true);
 const changes=[['STOP_DISABLED',header.replace("if(ch=='x'){reset();return event(kStop);}","if(ch=='x'){return event(kNone);}"),body,app],['QUIET_41',header.replace('kQuietMs=40','kQuietMs=41'),body,app],['EEPROM_DISABLED',header,body,app.replace('eepromWriteInt(i*2,menu[i].value);','/*mutant disabled write*/;')],['STEP_AFTER_STOP',header,body.replace('uint8_t budget=32;','if(usb_start) pump(delay_us);\n uint8_t budget=32;'),app]];
 for(const [name,h,b,a] of changes){if(h===header&&b===body&&a===app)throw Error('MUTANT_NOOP '+name);trial(b,a,h,name,false);}
 console.log('P002_COMBINED_GATE=PASS; EEPROM_TIME=NOT_RUN; PHYSICAL_STOP=NOT_RUN; FRAMING=REVIEW; RELEASE_BLOCKED');
}catch(e){console.error('P002_COMBINED_GATE=BLOCKED '+e.message);process.exitCode=2;}
