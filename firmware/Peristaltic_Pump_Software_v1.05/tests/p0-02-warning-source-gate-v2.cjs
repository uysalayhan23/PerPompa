#!/usr/bin/env node
'use strict';
// Exact-source differential checks and 4 negative source mutants.
const fs=require('node:fs'),path=require('node:path');
const {build,replacements}=require('./p0-02-warning-candidate-v2.cjs');
const root=path.resolve(__dirname,'..');
function audit(newSource,oldSource){
 if(!newSource.includes('#include <string.h>'))throw Error('STRING_HEADER_MISSING');
 if(newSource.includes('.suffix=="'))throw Error('POINTER_COMPARE_REMAINS');
 for(const [oldValue,newValue,n] of replacements){
  if(newSource.split(newValue).length-1!==n)throw Error('WARNING_FIX_COUNT_'+oldValue);
 }
 if(!newSource.includes('case ClickEncoder::Open:\n      case ClickEncoder::Pressed:\n        break;'))
   throw Error('ENCODER_ENUM_NOOP_MISSING');
 if(newSource.split('if (!hwMotorAllowed())').length-1!==2)
   throw Error('PROTECTED_STEP_GUARDS_CHANGED');
 let expected=oldSource;
 for(const [from,to] of replacements)expected=expected.split(from).join(to);
 if(expected!==newSource)throw Error('UNINTENDED_FIRMWARE_CHANGE');
 if(!newSource.includes('perpompa_usb::Parser usbParser;'))
   throw Error('ACTIVE_USB_PARSER_CHANGED');
 return 8;
}
try{
 const oldSource=fs.readFileSync(path.join(root,'Peristaltic_Pump_Software_v1.05.ino'),'utf8');
 const candidate=build();
 const n=audit(candidate,oldSource);
 const mutants=[
  candidate.replace('strcmp(menu[1].suffix,"uL")==0','menu[1].suffix=="uL"'),
  candidate.replace('strcmp(menu[3].suffix,"uL/min")==0','menu[3].suffix=="uL/min"'),
  candidate.replace('case ClickEncoder::Open:\n      case ClickEncoder::Pressed:\n        break;',''),
  candidate.replace('if (!hwMotorAllowed())','if (true)')
 ];
 for(const mutant of mutants){
  let blocked=false;try{audit(mutant,oldSource);}catch(e){blocked=true;}
  if(!blocked)throw Error('NEGATIVE_MUTANT_ESCAPED');
 }
 console.log('P002_WARNING_V2_SOURCE='+n+'/'+n+
   ' PASS; MUTANTS=4/4 BLOCKED; ACTIVE_INO_UNCHANGED; FULL_CPP=NOT_RUN; AVR=NOT_RUN');
}catch(e){console.error('P002_WARNING_SOURCE=BLOCKED '+e.message);process.exitCode=2;}
