#!/usr/bin/env node
'use strict';
// Extracts exact selected functions and local/USB action dispatch from the candidate generated
// by p0-01-candidate-dual-build.cjs, runs native host C++ tests with mock GPIO.
// NOT full loop, not physical motor, not AVR execution, not a safety release.
const fs=require('node:fs'), path=require('node:path'), os=require('node:os'), cp=require('node:child_process');
const base=path.resolve(__dirname,'..');
const sketch=fs.readFileSync(path.join(base,'Peristaltic_Pump_Software_v1.05.ino'),'utf8');
const gen=fs.readFileSync(path.join(__dirname,'p0-01-candidate-dual-build.cjs'),'utf8');
function jsonAssignment(name){
 const match=gen.match(new RegExp('^ const '+name+'=(\\[[^\\r\\n]+\\]);$','m'));
 if(!match)throw Error('Missing candidate patch list: '+name);
 return JSON.parse(match[1]);
}
function replaceFunction(text,signature,replacement){
 const pos=text.indexOf(signature);
 if(pos<0 || text.indexOf(signature,pos+signature.length)>=0)throw Error('Ambiguous '+signature);
 const start=text.indexOf('{',pos);let depth=0;
 for(let i=start;i<text.length;i++){
  if(text[i]==='{')depth++;
  if(text[i]==='}' && !--depth)return text.slice(0,pos)+replacement+text.slice(i+1);
 }
 throw Error('Unbalanced '+signature);
}
function extract(text,signature){
 const start=text.indexOf(signature);
 if(start<0||text.indexOf(signature,start+signature.length)>=0)throw Error('Ambiguous '+signature);
 const open=text.indexOf('{',start);let depth=0;
 for(let i=open;i<text.length;i++){
  if(text[i]==='{')depth++;
  if(text[i]==='}' && !--depth)return text.slice(start,i+1);
 }
 throw Error('Unbalanced extraction '+signature);
}
let source=sketch;
for(const [sig,body] of jsonAssignment('functions'))source=replaceFunction(source,sig,body);
for(const [from,to] of jsonAssignment('anchors')){
 if(source.split(from).length!==2)throw Error('Ambiguous candidate anchor '+from.slice(0,45));
 source=source.replace(from,to);
}
const case0=source.match(/case 0: \/\/Start([\s\S]*?)  break;\s*case 8:/);
if(!case0)throw Error('Start case extraction failed');
const usbStart=source.indexOf('  if (usb_start) {',source.indexOf('case 9:'));
if(usbStart<0)throw Error('USB action extraction failed');
const usbBlock=extract(source.slice(usbStart),'if (usb_start) {');
const prefix=String.raw`
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#define STEP_MODE 4
#define STEPS_PER_FULL_ROT 200
#define CALIBR_ROTATIONS 30
#define CALIBR_DECIMALS 3
#define MICROSEC_PER_SEC 1000000
#define MOTOR_STEP_PIN 7
#define HIGH 1
#define LOW 0
using boolean=bool;
const int CALIBR_DECIMAL_CORR=1000;
const unsigned CALIBR_STEPS=24000, CALIBR_DELAY_US=625;
int highs=0,lows=0,us_delays=0,rng_changes=0;
bool rng_running=false,in_action=false,usb_start=false;
long steps=0,delay_us=0,step_counter=0;char inChar='\0';
struct MenuItem {int value;}; MenuItem menu[11]{};
int &mode=menu[6].value;
void digitalWrite(int pin,int value){if(pin==7){if(value==HIGH)highs++;else lows++;}}
void delayMicroseconds(unsigned int){us_delays++;}
void set_rng_running(bool running){if(rng_running!=running){rng_running=running;rng_changes++;}}
void exit_action_menu(){set_rng_running(false);in_action=false;}
`;
const tests=String.raw`
int count=0;
void check(bool ok,const char* label){
 if(!ok){std::fprintf(stderr,"HOST_CPP_FAIL %d %s\n",count+1,label);std::exit(1);}
 count++;
}
void reset(){
 highs=lows=us_delays=rng_changes=0;rng_running=false;in_action=false;usb_start=false;
 steps=delay_us=step_counter=0;mode=0;inChar='\0';
}
int main(){
 check(steps_calc(10,0,5000,1)==4800,"normal dose");
 check(delay_us_calc(100,0,5000,1)==625,"normal speed");
 check(steps_calc(3,2,0,0)==2400,"rotation dose without calibration");
 check(delay_us_calc(10,2,0,0)==3750,"rpm without calibration");
 for(int unit=0;unit<3;unit++){
  check(steps_calc(0,unit,5000,1)==0,"zero volume per unit");
  check(steps_calc(-1,unit,5000,1)==0,"negative volume per unit");
  check(delay_us_calc(0,unit,5000,1)==0,"zero rate per unit");
  check(delay_us_calc(-1,unit,5000,1)==0,"negative rate per unit");
 }
 check(steps_calc(10,-1,5000,1)==0,"unit -1");
 check(steps_calc(10,3,5000,1)==0,"unit 3");
 check(delay_us_calc(10,-1,5000,1)==0,"rate unit -1");
 check(delay_us_calc(10,3,5000,1)==0,"rate unit 3");
 check(steps_calc(10,0,0,1)==0,"zero calibration steps");
 check(delay_us_calc(10,0,0,1)==0,"zero calibration rate");
 check(steps_calc(9999,0,1,1)==0,"signed 32-bit overflow guard");
 check(delay_us_calc(3,0,5000,1)==0,"overslow delay guard");
 check(steps_calc(10,0,5000,-1)==0,"negative decimals");
 check(delay_us_calc(10,0,5000,4)==0,"large decimals");
 reset();mode=0;steps=2;delay_us=625;in_action=true;
 localActionTick();check(highs==1&&lows==1&&rng_running&&step_counter==1,"local dose first");
 localActionTick();check(highs==2&&lows==2&&rng_running&&step_counter==2,"local dose second");
 localActionTick();check(highs==2&&!rng_running&&!in_action&&step_counter==0,"local dose completion");
 reset();mode=0;steps=0;delay_us=625;in_action=true;localActionTick();
 check(highs==0&&!rng_running&&!in_action,"local zero volume");
 reset();mode=0;steps=2;delay_us=0;in_action=true;localActionTick();
 check(highs==0&&!rng_running&&!in_action,"local zero speed");
 reset();mode=1;delay_us=625;in_action=true;localActionTick();
 check(highs==1&&rng_running&&in_action,"local pump normal");
 reset();mode=1;delay_us=0;in_action=true;localActionTick();
 check(highs==0&&!rng_running&&!in_action,"local pump zero delay");
 reset();mode=1;delay_us=16001;in_action=true;localActionTick();
 check(highs==0&&!rng_running&&!in_action,"local pump excessive delay");
 reset();mode=2;in_action=true;localActionTick();
 check(highs==1&&rng_running&&step_counter==1,"calibration first pulse");
 reset();inChar='p';usb_start=true;delay_us=625;usbActionTick();
 check(highs==1&&rng_running&&usb_start,"USB pump normal");
 reset();inChar='p';usb_start=true;delay_us=0;usbActionTick();
 check(highs==0&&!rng_running&&!usb_start,"USB pump invalid");
 reset();inChar='d';usb_start=true;steps=1;delay_us=625;usbActionTick();
 check(highs==1&&rng_running&&usb_start&&step_counter==1,"USB dose normal");
 usbActionTick();check(highs==1&&!rng_running&&!usb_start,"USB dose complete");
 reset();inChar='d';usb_start=true;steps=0;delay_us=625;usbActionTick();
 check(highs==0&&!rng_running&&!usb_start,"USB dose zero volume");
 reset();inChar='d';usb_start=true;steps=1;delay_us=0;usbActionTick();
 check(highs==0&&!rng_running&&!usb_start,"USB dose zero rate");
 reset();inChar='c';usb_start=true;usbActionTick();
 check(highs==1&&rng_running&&step_counter==1,"USB calibration");
 std::printf("SOURCE_EXTRACTED_BRANCH_HOST_CPP=%d/%d PASS; FULL_SKETCH_LOOP=NOT_RUN; HW=NOT_RUN\n",count,count);
}
`;
const functions=['long steps_calc(','long delay_us_calc(','boolean dose(','void pump('].map(sig=>extract(source,sig)).join('\n');
const testCode=prefix+functions+
 '\nvoid localActionTick(){if(!in_action)return;\n'+case0[1]+'\n}\n'+
 '\nvoid usbActionTick(){\n'+usbBlock+'\n}\n'+tests;
const tmp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-real-branch-'));
try {
 const file=path.join(tmp,'test.cpp'),binary=path.join(tmp,'test');
 fs.writeFileSync(file,testCode);
 const gcc=cp.spawnSync(process.env.CXX||'g++',['-std=c++17','-O0','-Wall','-Wextra','-Werror',
  '-fsanitize=undefined,float-cast-overflow','-fno-sanitize-recover=all',file,'-o',binary],
  {encoding:'utf8',timeout:30000});
 if(gcc.error||gcc.status!==0){console.error('HOST_CPP_COMPILE_FAIL',gcc.error?.message||gcc.stderr);process.exitCode=2;}
 else {
  const run=cp.spawnSync(binary,[],{encoding:'utf8',timeout:30000});
  if(run.error||run.status!==0){console.error('HOST_CPP_TEST_FAIL',run.error?.message||run.stderr||run.stdout);process.exitCode=1;}
  else console.log(run.stdout.trim());
 }
 console.log('SOURCE=UNMODIFIED; A4988/DRV8825_HW=NOT_RUN; FULL_LOOP=NOT_RUN');
}finally{fs.rmSync(tmp,{recursive:true,force:true});}
