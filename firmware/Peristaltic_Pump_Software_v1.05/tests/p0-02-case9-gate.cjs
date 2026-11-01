#!/usr/bin/env node
'use strict';
// Execute extracted production case 9 code, never a handwritten UART loop.
// Mock timing is not physical STOP latency.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),crypto=require('node:crypto'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const firmwarePath=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
const parserPath=path.join(root,'p0-02-usb-parser.h');
const templatePath=path.join(__dirname,'case9-native-template.cpp');
const expectFirmware='fd7a78a0ec4aae33eb466dc1cdaae671fa3ca02f';
const expectParser='20527923030414d7d385b5102b9307446ce7f5c7';
function sha(buf){return crypto.createHash('sha1').update('blob '+buf.length+'\0').update(buf).digest('hex');}
function protectedSource(file,expected){const b=fs.readFileSync(file);if(sha(b)===expected)return b.toString('utf8');const normalized=Buffer.from(b.toString('utf8').replace(/\r\n/g,'\n'),'utf8');if(sha(normalized)===expected)return normalized.toString('utf8');throw Error('SOURCE_SHA_CHANGED '+path.basename(file));}
function extractCase9(source){
 const s=source.replace(/\r\n/g,'\n');const matches=[...s.matchAll(/^\s*case 9: \{/gm)];
 if(matches.length!==1)throw Error('CASE9_ANCHOR_COUNT '+matches.length);
 const start=matches[0].index;
 let depth=0,end=-1;
 // Closing brace after the case's break; all braces in this case are code braces.
 for(let i=s.indexOf('{',start);i<s.length;i++){if(s[i]==='{')depth++;else if(s[i]==='}'&&--depth===0){end=i+1;break;}}
 if(end<0)throw Error('CASE9_UNBALANCED');
 const result=s.slice(start,end);
 if(!result.includes('uint8_t budget=32;')||!result.includes('usbParser.feed')||!result.includes('usbParser.tick')||!result.includes('hwForceIdle()'))throw Error('CASE9_INVARIANTS');
 return result;
}
function execute(source,template,parser,tag,expectPass){
 const base=fs.mkdtempSync(path.join(os.tmpdir(),'p002-case9-'));
 try{fs.writeFileSync(path.join(base,'p0-02-usb-parser.h'),parser);const cpp=template.replace('/*CASE9_INSERT*/',source);if(cpp===template)throw Error('TEMPLATE_ANCHOR');const file=path.join(base,'test.cpp');fs.writeFileSync(file,cpp);
 for(const hz of [16000000,8000000]){
 const exe=path.join(base,'test-'+hz+(process.platform==='win32'?'.exe':''));const flags=['-std=c++11','-Wall','-Wextra','-O0','-DF_CPU='+hz, '-I',base,file,'-o',exe];const c=cp.spawnSync(process.env.CXX||'g++',flags,{encoding:'utf8',timeout:60000});if(c.error||c.status!==0)throw Error('COMPILE_'+tag+' '+hz+' '+(c.stderr||c.error));
 const x=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000});const success=!x.error&&x.status===0&&(x.stdout||'').includes('P002_CASE9_NATIVE_FCPU_'+hz+'=13/13 PASS');
 if(success!==expectPass)throw Error('CASE9_'+tag+'_'+hz+' '+JSON.stringify({status:x.status,stdout:x.stdout,stderr:x.stderr}));
 console.log('P002_CASE9_'+tag+'_'+hz+'='+(expectPass?'13/13 PASS':'BLOCKED'));
 }
 }finally{fs.rmSync(base,{recursive:true,force:true});}
}
try{
 const fw=protectedSource(firmwarePath,expectFirmware),parser=protectedSource(parserPath,expectParser),template=fs.readFileSync(templatePath,'utf8');let src=extractCase9(fw);
 execute(src,template,parser,'PRODUCTION',true);
 const mutants=[['BUDGET_33',src.replace('uint8_t budget=32;','uint8_t budget=33;')],['QUIET_41',src],['STOP_AFTER_STEP',src.replace('uint8_t budget=32;','if(usb_start) pump(delay_us);\n    uint8_t budget=32;')]];
 // Mutate the REAL parser's quiet timeout, rather than the case body.
 if(!parser.includes('kQuietMs=40'))throw Error('PARSER_ANCHOR');
 for(const [name,mutant] of mutants){if(name!=='QUIET_41'&&mutant===src)throw Error('MUTANT_NOOP '+name);execute(mutant,template,name==='QUIET_41'?parser.replace('kQuietMs=40','kQuietMs=41'):parser,name,false);}
 console.log('P002_CASE9_SOURCE_GATE=PASS; EEPROM_TIMING=NOT_RUN; PHYSICAL_STOP=NOT_RUN; FRAMING=REVIEW; RELEASE_BLOCKED');
}catch(e){console.error('P002_CASE9_GATE=BLOCKED '+e.message);process.exitCode=2;}