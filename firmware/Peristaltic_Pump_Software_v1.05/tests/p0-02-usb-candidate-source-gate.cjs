#!/usr/bin/env node
'use strict';
// Static differential inspection of the SHA-locked temporary P0-02 USB candidate.
// This never certifies real AVR timing, or full setup()/loop() behavior.
const fs=require('node:fs'),path=require('node:path');
const {build}=require('./p0-02-usb-candidate.cjs');
const root=path.resolve(__dirname,'..');
function body(s,sig){
 const i=s.indexOf(sig);if(i<0||s.indexOf(sig,i+sig.length)!==-1)throw Error('ANCHOR_'+sig);
 let depth=0;for(let j=s.indexOf('{',i);j<s.length;j++){
  if(s[j]==='{')depth++;else if(s[j]==='}'&&!--depth)return s.slice(i,j+1);
 }throw Error('UNBALANCED_'+sig);
}
function audit(src,old){
 const need=[
  '#include "p0-02-usb-parser.h"',
  'perpoma_usb_placeholder',
  'perpoma_usb_placeholder2'
 ];
 need[1]='perpompa_usb::Parser usbParser;';
 need[2]='perpompa_usb::Event idleEvent=usbParser.tick(millis());';
 need.push('usbParser.feed((uint8_t)Serial.read(),millis());',
           'if (e.kind!=perpompa_usb::kNone) usbApplyEvent(e);',
           'case perpompa_usb::kStop:',
           'case perpompa_usb::kReject:',
           'case perpompa_usb::kPending:',
           'hwForceIdle(); inChar=0; step_counter=0; return;',
           'if (usbDropUntilEmpty) { (void)Serial.read(); continue; }',
           'usbDropUntilEmpty=true; hwForceIdle(); inChar=0;',
           'void exit_action_menu(){\n   usbParser.reset();',
           'case perpompa_usb::kWriteCalibration:',
           'if(e.value[0]<=0 || e.value[0]>menu[7].lim)return;',
           'inChar=\'c\'; step_counter=0; usb_start=true;',
           'if (usb_start) {',
           'hwSetup.permit(path,false)',
           'hwSetup.permit(static_cast<perpompa_hw::StartPath>');
 for(const marker of need)if(!src.includes(marker))throw Error('SAFETY_MARKER_MISSING '+marker);
 if(src.includes('Serial.parseInt()'))throw Error('BLOCKING_PARSE_LEFT');
 if((src.match(/if \(!hwMotorAllowed\(\)\)/g)||[]).length!==2)
  throw Error('MOTOR_PULSE_GUARD_CHANGED');
 const sigs=['void pump(int _delay_us) {',
 'boolean dose(long _steps, int _delay_us, long & inc) {',
 'void handle_remote_input(){',
 'bool hwMotorAllowed()'];
 for(const sig of sigs)if(body(src,sig)!==body(old,sig))throw Error('PROTECTED_FUNCTION_CHANGED '+sig);
 if((src.match(/for \(int i=0; i < 11; i\+\+\)/g)||[]).length!==2)
  throw Error('LEGACY_EEPROM_LOOP_CHANGED');
 if(!src.includes('delay(700)'))throw Error('SAVE_PATH_CHANGED');
 return need.length+8;
}
try{
 const old=fs.readFileSync(path.join(root,'Peristaltic_Pump_Software_v1.05.ino'),'utf8');
 const src=build();
 const count=audit(src,old);
 const mutants=[
  src.replace('case perpompa_usb::kStop:','case perpompa_usb::kNone_STOP_BROKEN:'),
  src.replace('if (!hwMotorAllowed())','if (true)'),
  src.replace('usbParser.feed((uint8_t)Serial.read(),millis());','usbParser.feed(0,millis());')
 ];
 for(const mutant of mutants){
  let blocked=false;try{audit(mutant,old);}catch(e){blocked=true;}
  if(!blocked)throw Error('NEGATIVE_MUTANT_ESCAPED');
 }
 console.log('P002_USB_CANDIDATE_SOURCE_CHECK='+count+'/'+count+
   ' PASS; MUTANTS=3/3 BLOCKED; ACTIVE_INO_UNCHANGED; FULL_CPP=NOT_RUN; AVR=NOT_RUN');
}catch(e){console.error('P002_USB_CANDIDATE_SOURCE=BLOCKED '+e.message);process.exitCode=2;}
