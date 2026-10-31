#!/usr/bin/env node
'use strict';
// Warning-fix candidate only. Never touches active firmware, never flashes hardware.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'..');
const accepted='04185aaa799b0af0bfa663b8acc8f5013a515066';
function build(){
 const b=fs.readFileSync(path.join(root,'Peristaltic_Pump_Software_v1.05.ino'));
 const sha=crypto.createHash('sha1').update('blob '+b.length).update(Buffer.from([0])).update(b).digest('hex');
 if(sha!==accepted)throw Error('P002_SOURCE_SHA_CHANGED');
 let source=b.toString('utf8');
 const fixes=[
  ['menu[1].suffix=="uL"','strcmp(menu[1].suffix,"uL")==0',2],
  ['menu[3].suffix=="uL/min"','strcmp(menu[3].suffix,"uL/min")==0',1],
  ['switch (b) {\n      case ClickEncoder::Clicked:',
   'switch (b) {\n      case ClickEncoder::Open:\n      case ClickEncoder::Pressed:\n        break;\n      case ClickEncoder::Clicked:',1]
 ];
 for(const [from,to,want]of fixes){
  const n=source.split(from).length-1;
  if(n!==want)throw Error('WARNING_ANCHOR_FAIL '+n+' vs '+want+' '+from);
  source=source.split(from).join(to);
 }
 if(source.includes('.suffix=="')||!source.includes('#include <string.h>'))
   throw Error('SUFFIX_FIX_INCOMPLETE');
 return source;
}
module.exports={build};
if(require.main===module){
 try{const src=build();console.log('P002_WARNINGS_CANDIDATE_READY source_chars='+src.length+
  '; POINTER_WARNINGS=3_FIXED_IN_TEMP; SWITCH_WARNINGS=1_FIXED_IN_TEMP; ACTIVE_INO_UNCHANGED; AVR=NOT_RUN');}
 catch(e){console.error('P002_WARNING_CANDIDATE=BLOCKED '+e.message);process.exitCode=2;}
}
