#!/usr/bin/env node
'use strict';
// P0-02 Source-derived NEGATIVE reproduction of synchronous USB command parsing.
// The millis estimates are from Arduino Stream's 1000ms DEFAULT timeout; NOT
// measured on a device. NEVER label this a STOP safety PASS.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const ino=path.join(__dirname,'..','Peristaltic_Pump_Software_v1.05.ino');
function body(s,sig){
 const at=s.indexOf(sig);if(at<0||s.indexOf(sig,at+sig.length)>=0)throw Error('SOURCE_ANCHOR_MISSING '+sig);
 let n=0;for(let i=s.indexOf('{',at);i<s.length;i++){
  if(s[i]==='{')n++;else if(s[i]==='}'&&!--n)return s.slice(at,i+1);
 }throw Error('UNBALANCED '+sig);
}
try{
 const bytes=fs.readFileSync(ino),sha=crypto.createHash('sha1')
  .update('blob '+bytes.length).update(Buffer.from([0])).update(bytes).digest('hex');
 if(sha!=='04185aaa799b0af0bfa663b8acc8f5013a515066')throw Error('P002_ACCEPTED_BLOB_CHANGED');
 const s=bytes.toString('utf8'),loop=body(s,'void loop()');
 const start=loop.indexOf('  case 9:'),end=loop.indexOf('/// MENU (no action)',start);
 if(start<0||end<=start)throw Error('USB_SOURCE_PATH_MISSING');
 const usb=loop.slice(start,end);
 const cmds=[
  ['p','if (inChar == \x27p\x27)',"else if (inChar == 'd')",2],
  ['d',"else if (inChar == 'd')","else if (inChar == 'c')",3],
  ['w',"else if (inChar == 'w')","else if (inChar == 'x')",1]
 ];
 if(loop.lastIndexOf('handle_remote_input();')<end||s.includes('Serial.setTimeout('))
  throw Error('STOP_OR_TIMEOUT_SOURCE_PATH_CHANGED');
 for(const [name,a,b,n] of cmds){
  const i=usb.indexOf(a),j=usb.indexOf(b,i+a.length);
  if(i<0||j<i)throw Error('USB_COMMAND_SOURCE_CHANGED '+name);
  const code=usb.slice(i,j),actual=(code.match(/Serial\.parseInt\(\)/g)||[]).length;
  if(actual!==n)throw Error('USB_PARSEINT_COUNT_CHANGED '+name);
  console.log('P002_TRUNCATED_USB_'+name+': '+actual+
   ' consecutive blocking parseInt calls; conditional default-timeout exposure up to '+(n*1000)+
   'ms without required delimiters/data (MODEL, NOT measured)');
 }
 if(!usb.includes("else if (inChar == 'x')")||
    !usb.includes('digitalWrite(MOTOR_STEP_PIN, LOW)'))throw Error('USB_STOP_X_SOURCE_CHANGED');
 console.log('P002_USB_BLOCKING_NEGATIVE_SOURCE_REPRO=PASS; STOP_X_BRANCH_EXISTS; READ_LOOP_CAN_BE_BLOCKED');
 console.log('P002_USB_STOP_LATENCY=REVIEW; NONBLOCKING_FIX=NOT_IMPLEMENTED; HW=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_USB_SOURCE_REPRO=BLOCKED '+e.message);process.exitCode=2;}
