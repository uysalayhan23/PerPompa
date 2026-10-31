#!/usr/bin/env node
'use strict';
// Generate TEMP sketch from SHA-locked accepted firmware. No active-file writes.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'..');
const accepted='04185aaa799b0af0bfa663b8acc8f5013a515066';
function gitSha(bytes){return crypto.createHash('sha1').update('blob '+bytes.length)
  .update(Buffer.from([0])).update(bytes).digest('hex');}
const usbCase=`  case 9: {
    // Bounded, nonblocking input. STOP events are processed before any STEP.
    uint8_t budget=32;
    while (budget && Serial.available()) {
      --budget;
      if (usbDropUntilEmpty) { (void)Serial.read(); continue; }
      perpompa_usb::Event e=usbParser.feed((uint8_t)Serial.read(),millis());
      if (e.kind!=perpompa_usb::kNone) usbApplyEvent(e);
    }
    if (usbDropUntilEmpty && !Serial.available()) usbDropUntilEmpty=false;
    if (Serial.available()) {
      // Fail closed on excess queued data; discard across future loops.
      usbParser.reset(); usbDropUntilEmpty=true; hwForceIdle(); inChar=0;
    }
    // Process UART input before quiet completion. An already-queued STOP or
    // late final digit must never be overtaken by a stale timeout event.
    if (!usbDropUntilEmpty && !Serial.available()) {
      perpompa_usb::Event idleEvent=usbParser.tick(millis());
      if (idleEvent.kind!=perpompa_usb::kNone) usbApplyEvent(idleEvent);
    }

`;
const eventFn=`void usbApplyEvent(const perpompa_usb::Event &e){
  switch(e.kind){
    case perpompa_usb::kNone: return;
    case perpompa_usb::kPending:
    case perpompa_usb::kStop:
    case perpompa_usb::kReject:
      hwForceIdle(); inChar=0; step_counter=0; return;
    case perpompa_usb::kCalibrate:
      hwForceIdle(); inChar='c'; step_counter=0; usb_start=true; return;
    case perpompa_usb::kPump: {
      if(e.value[0]<=0 || e.value[1]<0 || e.value[1]>20000) {
        hwForceIdle(); inChar=0; return;
      }
      int calibration=(int)(e.value[1] ? e.value[1] : menu[7].value);
      long d=delay_us_calc(e.value[0],1,calibration,0);
      if(d<4 || d>16000){hwForceIdle();inChar=0;return;}
      hwForceIdle();rate_uL_min=e.value[0];cal=calibration;delay_us=d;
      inChar='p'; step_counter=0;usb_start=true; return;
    }
    case perpompa_usb::kDose: {
      if(e.value[0]<=0 || e.value[1]<=0 || e.value[2]<0 || e.value[2]>20000){
        hwForceIdle();inChar=0;return;
      }
      int calibration=(int)(e.value[2] ? e.value[2] : menu[7].value);
      long stepCount=steps_calc(e.value[0],1,calibration,0);
      long d=delay_us_calc(e.value[1],1,calibration,0);
      if(stepCount<=0 || d<4 || d>16000){hwForceIdle();inChar=0;return;}
      hwForceIdle();vol_uL=e.value[0];rate_uL_min=e.value[1];cal=calibration;
      steps=stepCount;delay_us=d;inChar='d';step_counter=0;usb_start=true;return;
    }
    case perpompa_usb::kWriteCalibration:
      // Preserve legacy w persistence while rejecting invalid calibration.
      hwForceIdle();inChar=0;
      if(e.value[0]<=0 || e.value[0]>menu[7].lim)return;
      cal=(int)e.value[0];menu[7].value=cal;
      for(int i=0;i<11;i++)eepromWriteInt(i*2,menu[i].value);
      return;
  }
  hwForceIdle(); inChar=0;
}
`;
function unique(s,a,b){const n=s.split(a).length-1;
 if(n!==1)throw Error('P002_USB_PATCH_ANCHOR='+n+' '+a.slice(0,80));
 return s.replace(a,b);
}
function build(){
 const bytes=fs.readFileSync(path.join(root,'Peristaltic_Pump_Software_v1.05.ino'));
 if(gitSha(bytes)!==accepted)throw Error('P002_ACCEPTED_FIRMWARE_SHA_CHANGED');
 const header=fs.readFileSync(path.join(root,'p0-02-usb-parser.h'));
 if(header.length<2000)throw Error('P002_PARSER_HEADER_MISSING');
 let s=bytes.toString('utf8');
 s=unique(s,'#include "hw-setup-runtime.h"','#include "hw-setup-runtime.h"\n#include "p0-02-usb-parser.h"');
 s=unique(s,'boolean usb_start=0;',
  'boolean usb_start=0;\nperpompa_usb::Parser usbParser;\nbool usbDropUntilEmpty=false;');
 const begin='  case 9:\n  while (Serial.available()) {';
 const end='\n  if (usb_start) {';
 const at=s.indexOf(begin),after=s.indexOf(end,at+begin.length);
 if(at<0 || after<=at || s.indexOf(begin,at+begin.length)>=0)throw Error('P002_USB_CASE_NOT_UNIQUE');
 const oldCase=s.slice(at,after);
 for(const required of ['Serial.parseInt()','else if (inChar == \'x\')',"else if (inChar == 'w')"])
   if(!oldCase.includes(required))throw Error('P002_LEGACY_COMMAND_BRANCH_MISSING '+required);
 s=s.slice(0,at)+usbCase+s.slice(after);
 s=unique(s,'  break;\n}\n\n/// MENU (no action)',
    '  break;\n  }\n}\n\n/// MENU (no action)');
 s=unique(s,'void exit_action_menu(){',
    'void exit_action_menu(){\n   usbParser.reset();\n   usbDropUntilEmpty=false;');
 s=unique(s,'void hwForceIdle(){',eventFn+'\nvoid hwForceIdle(){');
 if(s.includes('Serial.parseInt()')||!s.includes('usbParser.tick(millis())'))
   throw Error('P002_BLOCKING_PARSE_LEFT');
 if((s.match(/if \(!hwMotorAllowed\(\)\)/g)||[]).length!==2)
   throw Error('P002_STEP_GUARD_COUNT_CHANGED');
 return s;
}
module.exports={build,eventFn};
if(require.main===module){
 try{const s=build();console.log('P002_USB_TEMP_CANDIDATE_READY chars='+s.length+
  '; ACTIVE_INO_UNCHANGED; FULL_LOOP=NOT_RUN; AVR=NOT_RUN; HW=NOT_RUN');}
 catch(e){console.error('P002_USB_CANDIDATE=BLOCKED '+e.message);process.exitCode=2;}
}
