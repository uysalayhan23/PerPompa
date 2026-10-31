#!/usr/bin/env node
'use strict';
// Source-locked native regression: a generated C++ prototype referring to
// perpompa_usb::Event must be placed AFTER the parser header is loaded.
// Neither the accepted .ino nor the hardware is modified.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),
  crypto=require('node:crypto'),cp=require('node:child_process');
const root=path.resolve(__dirname,'..');
const ino=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
const parser=path.join(root,'p0-02-usb-parser.h');
const qa=path.join(__dirname,'p0-02-usb-full-loop-avr-qa.cjs');
const expectedFirmware='04185aaa799b0af0bfa663b8acc8f5013a515066';
const expectedParser='20527923030414d7d385b5102b9307446ce7f5c7';
function blob(file){const bytes=fs.readFileSync(file);return crypto.createHash('sha1')
  .update('blob '+bytes.length).update(Buffer.from([0])).update(bytes).digest('hex');}
let temp;
try {
  if(blob(ino)!==expectedFirmware||blob(parser)!==expectedParser)
    throw Error('PROTECTED_SOURCE_SHA_MISMATCH');
  const text=fs.readFileSync(qa,'utf8');
  const prelude="const headerPrelude='#include \"p0-02-usb-parser.h\"\\n';";
  if(text.split(prelude).length!==2||!text.includes("stub+'\\n'+headerPrelude+prototypes+'\\n'+sketch+'\\n'+tests"))
    throw Error('WHOLE_LOOP_HEADER_BEFORE_PROTOTYPES_MISSING');
  temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-p002-prototypes-'));
  const cxx=process.env.CXX||'g++';
  for(const hz of [16000000,8000000]){
    const bad=path.join(temp,'bad-'+hz+'.cpp');
    const good=path.join(temp,'good-'+hz+'.cpp');
    const signature='void usbApplyEvent(const perpompa_usb::Event &e);\n';
    const definition='void usbApplyEvent(const perpompa_usb::Event &e){(void)e;}\nint main(){return 0;}\n';
    fs.writeFileSync(bad,signature+'#include "p0-02-usb-parser.h"\n'+definition);
    fs.writeFileSync(good,'#include "p0-02-usb-parser.h"\n'+signature+definition);
    const flags=['-std=c++11','-Wall','-Wextra','-Werror','-pedantic','-DF_CPU='+hz+'UL','-I',root,'-fsyntax-only'];
    const run=(file)=>cp.spawnSync(cxx,[...flags,file],{encoding:'utf8',timeout:60000});
    const negative=run(bad);
    if(negative.error||negative.status===0)
      throw Error('NEGATIVE_PROTOTYPE_MUTANT_NOT_DETECTED_'+hz+' '+(negative.error?.message||''));
    const positive=run(good);
    if(positive.error||positive.status!==0)
      throw Error('CORRECTED_PROTOTYPE_COMPILE_FAIL_'+hz+' '+(positive.error?.message||positive.stderr||''));
    console.log('P002_PROTO_HEADER_ORDER_'+hz+'=PASS; WRONG_ORDER_REJECTED; CORRECT_ORDER_ACCEPTED');
  }
  console.log('P002_PROTO_HEADER_NATIVE=2/2 PASS; FULL_LOOP_CPP=NOT_RUN; AVR=NOT_RUN; HW=NOT_RUN');
}catch(e){console.error('P002_PROTO_HEADER_NATIVE=BLOCKED '+e.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
