#!/usr/bin/env node
'use strict';
// Native USB event dispatch with real candidate function body and accepted
// pump/dose/Remote source-excerpts. Not complete setup()/loop() or AVR proof.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),cp=require('node:child_process');
const {eventFn}=require('./p0-02-usb-candidate.cjs');
const root=path.resolve(__dirname,'..');
const excerptFile=path.join(__dirname,'p0-02-targeted-source.cpp');
const sourceFile=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
function body(s,key){const i=s.indexOf(key);if(i<0||s.indexOf(key,i+key.length)>=0)throw Error('ANCHOR_UNIQUE_FAIL '+key);
 let depth=0;for(let j=s.indexOf('{',i);j<s.length;j++){
 if(s[j]==='{')depth++;else if(s[j]==='}'&&!--depth)return s.slice(i,j+1);
 }throw Error('SOURCE_UNBALANCED '+key);}
const signatures=['void pump(int _delay_us) {','boolean dose(long _steps, int _delay_us, long & inc) {','void handle_remote_input(){'];
const prelude=`#include "p0-02-usb-parser.h"
#include <cstring>
long rate_uL_min=0,vol_uL=0,steps=0,step_counter=0;
int cal=0, eepromWrites=0;char inChar=0;
void eepromWriteInt(int, int){eepromWrites++;}
long steps_calc(long volume,int,int calibration,int){return volume>0&&calibration>0?volume*2:0;}
long delay_us_calc(long rate,int,int calibration,int){return rate>0&&calibration>0?6250:0;}
`;
// Keep C++ cases separate for direct host review; eventFn is read from the
// SAME JS literal the candidate builder embeds into the future sketch.
const main=`
using namespace perpompa_usb;
static int checks=0;
void verifyUsb(bool ok,const char* text){if(!ok){std::fprintf(stderr,"P002_DISPATCH_FAIL %s\\n",text);std::exit(1);}++checks;}
void feed(Parser &parser,const char* text,uint32_t time){for(const char* c=text;*c;c++){
 Event e=parser.feed((uint8_t)*c,time++);if(e.kind!=kNone)usbApplyEvent(e);
}}
int main(){
  menu[7].value=5000;menu[7].lim=20000;pins[MOTOR_STEP_PIN]=LOW;
  Parser parser;
  // Existing motor RUN must cease at the first byte of a new partial command.
  usb_start=true;rng_running=true;pins[MOTOR_STEP_PIN]=HIGH;
  usbApplyEvent(parser.feed('p',100));
  verifyUsb(!usb_start&&!rng_running&&pins[MOTOR_STEP_PIN]==LOW,"pending command kills stale pump");
  feed(parser,"1000 5000",101);
  verifyUsb(!usb_start&&highEvents==0,"incomplete final field cannot step");
  usbApplyEvent(parser.tick(150));
  verifyUsb(usb_start&&inChar=='p'&&delay_us==6250,"complete legacy pump accepted after quiet gap");
  allow=true;pump((int)delay_us);
  verifyUsb(highEvents==1&&rng_running,"authorized pump STEP only after full valid packet");
  usbApplyEvent(parser.feed('x',151));
  verifyUsb(!usb_start&&!rng_running&&pins[MOTOR_STEP_PIN]==LOW,"USB x immediate stop");
  feed(parser,"d10 1000 5000\\n",200);
  verifyUsb(usb_start&&inChar=='d'&&steps==20,"dose only after three arguments");
  long inc=0;dose(steps,(int)delay_us,inc);
  verifyUsb(inc==1&&highEvents==2,"dose path still guarded");
  usbApplyEvent(parser.feed('p',300));feed(parser,"1000 ",301);
  usbApplyEvent(parser.tick(360));
  verifyUsb(!usb_start&&!rng_running&&pins[MOTOR_STEP_PIN]==LOW,"missing field rejects and stops");
  feed(parser,"p2147483648 ",400);
  verifyUsb(!usb_start&&!rng_running,"overflow fail closed");
  feed(parser,"p-4 100",500);
  verifyUsb(!usb_start,"malformed negative fail closed");
  usbApplyEvent(parser.feed('c',600));
  verifyUsb(usb_start&&inChar=='c',"calibration command preserved");
  usbApplyEvent(parser.feed('x',601));
  verifyUsb(!usb_start,"calibration stop x preserved");
  feed(parser,"w0 ",700);
  verifyUsb(eepromWrites==0&&menu[7].value==5000,"invalid w zero does not write");
  feed(parser,"w6000\\r",800);
  verifyUsb(eepromWrites==11&&menu[7].value==6000&&!usb_start,"valid w persists after parsing");
  usb_start=true;rng_running=true;usbApplyEvent(parser.feed('p',900));
  usbApplyEvent(parser.feed('x',901));
  verifyUsb(!usb_start&&!rng_running,"x preempts partial command");
  allow=false;usbApplyEvent(parser.feed('p',1000));feed(parser,"1000 6000 ",1001);
  const int before=highEvents;pump((int)delay_us);
  verifyUsb(highEvents==before&&!rng_running&&pins[MOTOR_STEP_PIN]==LOW,"HW guard rejects STEP");
  // Independent legacy Remote STOP body is source-identical; still debounced.
  allow=true;remote_armed=true;remote_started=true;in_action=true;
  menu[10].value=REMOTE_HOLD;remote_stable_state=LOW;remote_last_read=LOW;
  pins[REMOTE_PIN]=LOW;mockMs=100;pins[REMOTE_PIN]=HIGH;
  handle_remote_input();mockMs=129;handle_remote_input();
  verifyUsb(remote_started,"remote hold stable interval still required");
  mockMs=130;handle_remote_input();
  verifyUsb(!remote_started&&!in_action&&!rng_running,"remote hold stop eventually handled");
  in_action=true;usb_start=true;rng_running=true;
  exit_action_menu();
  verifyUsb(!in_action&&!usb_start&&!rng_running,"local stop resets state");
  std::printf("P002_USB_DISPATCH_FCPU_%lu=%d/%d PASS; SOURCE_EXCERPT; FULL_LOOP_NOT_RUN; HW_NOT_RUN\\n",
   (unsigned long)F_CPU,checks,checks);
}
`;
let temp;
try{
 if(!fs.existsSync(excerptFile)||!fs.existsSync(sourceFile)||!fs.existsSync(path.join(root,'p0-02-usb-parser.h')))
  throw Error('DISPATCH_INPUT_MISSING');
 const source=fs.readFileSync(sourceFile,'utf8'),excerpt=fs.readFileSync(excerptFile,'utf8');
 for(const sig of signatures)if(body(source,sig)!==body(excerpt,sig))throw Error('ACCEPTED_FUNCTION_BODY_CHANGED '+sig);
 let base=excerpt.slice(0,excerpt.indexOf('void check(bool ok,const char *s)'));
 if(!base.includes('struct Menu{int value=0;}'))throw Error('MOCK_MENU_ANCHOR_CHANGED');
 base=base.replace('struct Menu{int value=0;}','struct Menu{int value=0;int lim=20000;}');
 const sourceCpp=base+'\n'+prelude+'\n'+eventFn+'\n'+main;
 temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-usb-dispatch-'));
 const cpp=path.join(temp,'source.cpp');fs.writeFileSync(cpp,sourceCpp);
 for(const fcpu of [16000000,8000000]){
  const exe=path.join(temp,'dispatch-'+fcpu+(process.platform==='win32'?'.exe':''));
  const build=cp.spawnSync(process.env.CXX||'g++',['-std=c++11','-O0','-Wall','-Wextra','-Werror',
    '-pedantic','-fsanitize=undefined','-fno-sanitize-recover=all','-DF_CPU='+fcpu+'UL',
    '-I',root,cpp,'-o',exe],{encoding:'utf8',timeout:60000,maxBuffer:2097152});
  if(build.stdout)process.stdout.write(build.stdout);
  if(build.stderr)process.stderr.write(build.stderr);
  if(build.error||build.status!==0)throw Error('DISPATCH_CXX_COMPILE_'+fcpu+' '+(build.error?.message||build.status));
  const run=cp.spawnSync(exe,[],{encoding:'utf8',timeout:60000});
  if(run.stdout)process.stdout.write(run.stdout);
  if(run.stderr)process.stderr.write(run.stderr);
  if(run.error||run.status!==0||!(run.stdout||'').includes('P002_USB_DISPATCH_FCPU_'+fcpu+'=19/19 PASS'))
   throw Error('DISPATCH_CXX_TEST_'+fcpu+' '+(run.error?.message||run.status));
 }
 console.log('P002_USB_DISPATCH_TWO_FCPU_PASS=2/2; REAL_EVENT_FUNCTION; AVR_NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_USB_DISPATCH=BLOCKED '+e.message);process.exitCode=2;}
finally{if(temp)fs.rmSync(temp,{recursive:true,force:true});}
