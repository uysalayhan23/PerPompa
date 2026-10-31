#!/usr/bin/env node
'use strict';
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),crypto=require('node:crypto'),cp=require('node:child_process');
const sketch=path.resolve(__dirname,'..','Peristaltic_Pump_Software_v1.05.ino');
const archive=path.resolve(__dirname,'..','..','archive','Peristaltic_Pump_Software_v1.04','Peristaltic_Pump_Software_v1.04.ino');
const baseline='0d965c08947ba0b3170d22fbff806fc2e6f1e83a';
function gitSha(file) {const b=fs.readFileSync(file);return crypto.createHash('sha1').update(Buffer.from('blob '+b.length+'\0')).update(b).digest('hex');}
function stop(reason){ console.error('P001_GATE_BLOCKED: '+reason);process.exitCode=2; }
function replaceFunction(text,sig,repl){
 const first=text.indexOf(sig);if(first<0||text.indexOf(sig,first+sig.length)>=0)throw Error('Signature not unique: '+sig);
 const a=text.indexOf('{',first);let depth=0;
 for(let i=a;i<text.length;i++){if(text[i]==='{')depth++;if(text[i]==='}'&&!--depth)return text.slice(0,first)+repl+text.slice(i+1)}
 throw Error('Unbalanced function '+sig);
}
function replaceOnce(text,from,to){if(text.split(from).length!==2)throw Error('Patch anchor not unique: '+from.slice(0,36));return text.replace(from,to)}
if(!fs.existsSync(sketch)||!fs.existsSync(archive)) {stop('Missing project files');}
else if(gitSha(sketch)!==baseline||gitSha(archive)!==baseline){stop('SOURCE_SHA_MISMATCH: re-evaluate candidate before use');}
else{
 let source=fs.readFileSync(sketch,'utf8');
 const functions=[["long steps_calc(","long steps_calc(long volume, int unit_mode, int calibr, int decimals){\n  if (volume <= 0 || unit_mode < 0 || unit_mode > 2 ||\n      decimals < 0 || decimals > 3 || (unit_mode != 2 && calibr <= 0)) return 0;\n  int decimal_corr = pow(10, decimals);\n  double cal = ((double)calibr / CALIBR_ROTATIONS) / CALIBR_DECIMAL_CORR;\n  double conv = unit_mode == 2 ? 1.0 : (unit_mode == 1 ? 1.0 / cal / 1000 : 1.0 / cal);\n  double candidate = (double)STEPS_PER_FULL_ROT * STEP_MODE * conv * volume / decimal_corr;\n  // On AVR double is 32-bit float. Keep a guard margin below signed-long overflow.\n  if (!(candidate >= 1.0 && candidate <= 2147483000.0)) return 0;\n  return (long)candidate;\n}"],["long delay_us_calc(","long delay_us_calc(long vol_per_min, int unit_mode, int calibr, int decimals){\n  if (vol_per_min <= 0 || unit_mode < 0 || unit_mode > 2 ||\n      decimals < 0 || decimals > 3 || (unit_mode != 2 && calibr <= 0)) return 0;\n  int decimal_corr = pow(10, decimals);\n  double cal = ((double)calibr / CALIBR_ROTATIONS) / CALIBR_DECIMAL_CORR;\n  double conv = unit_mode == 2 ? 1.0 : (unit_mode == 1 ? 1.0 / cal / 1000 : 1.0 / cal);\n  double candidate = (1.0 / (STEPS_PER_FULL_ROT * STEP_MODE * conv *\n                     vol_per_min / decimal_corr)) * 60 * MICROSEC_PER_SEC / 2;\n  // Fail closed: AVR int is 16-bit and overly long delayMicroseconds is not supported.\n  if (!(candidate >= 4.0 && candidate <= 16000.0)) return 0;\n  return (long)candidate;\n}"],["boolean dose(","boolean dose(long _steps, int _delay_us, long & inc) {\n  if (_steps <= 0 || _delay_us < 4 || _delay_us > 16000) {\n    digitalWrite(MOTOR_STEP_PIN, LOW);\n    set_rng_running(false);\n    inc = 0;\n    return true;\n  }\n  if (inc < _steps) {\n    set_rng_running(true);\n    digitalWrite(MOTOR_STEP_PIN, HIGH);\n    delayMicroseconds(_delay_us);\n    digitalWrite(MOTOR_STEP_PIN, LOW);\n    delayMicroseconds(_delay_us);\n    inc++;\n    return false;\n  }\n  set_rng_running(false);\n  inc=0;\n  return true;\n}"],["void pump(","void pump(int _delay_us) {\n  if (_delay_us < 4 || _delay_us > 16000) {\n    digitalWrite(MOTOR_STEP_PIN, LOW);\n    set_rng_running(false);\n    return;\n  }\n  set_rng_running(true);\n  digitalWrite(MOTOR_STEP_PIN, HIGH);\n  delayMicroseconds(_delay_us);\n  digitalWrite(MOTOR_STEP_PIN, LOW);\n  delayMicroseconds(_delay_us);\n}"]];
 const anchors=[["if (menu[6].value == 0){ //Dose\n    if (dose(steps, delay_us, step_counter)){","if (menu[6].value == 0){ //Dose\n    if (steps <= 0 || delay_us < 4 || delay_us > 16000 || dose(steps, delay_us, step_counter)){"],["} else if (menu[6].value == 1){ //Pump\n    pump(delay_us);","} else if (menu[6].value == 1){ //Pump\n    if (delay_us < 4 || delay_us > 16000) exit_action_menu();\n    else pump(delay_us);"],["if (usb_start) {\n    if(inChar == 'p'){","if (usb_start) {\n    if ((inChar == 'p' && (delay_us < 4 || delay_us > 16000)) ||\n        (inChar == 'd' && (steps <= 0 || delay_us < 4 || delay_us > 16000))) {\n      usb_start = false;\n      set_rng_running(false);\n    } else if(inChar == 'p'){"]];
 for(const [sig,replacement] of functions)source=replaceFunction(source,sig,replacement);
 for(const [from,to] of anchors)source=replaceOnce(source,from,to);
 const folder=fs.mkdtempSync(path.join(os.tmpdir(),'PerPompa_P001_'));
 const target=path.join(folder,'Peristaltic_Pump_Software_v1.05');
 fs.mkdirSync(target);
 const output=path.join(target,'Peristaltic_Pump_Software_v1.05.ino');
 fs.writeFileSync(output,source);
 const cli=process.env.ARDUINO_CLI||'arduino-cli';
 let pass=0;
 try{
  for(const freq of ['16MHzatmega328','8MHzatmega328']){
   const fqbn='arduino:avr:pro:cpu='+freq;
   const args=['compile','--fqbn',fqbn,'--build-path',path.join(folder,'build_'+freq),target];
   const r=cp.spawnSync(cli,args,{encoding:'utf8',timeout:180000,maxBuffer:4*1024*1024});
   console.log('TARGET: '+fqbn);
   if(r.stdout)console.log(r.stdout.trim());
   if(r.error||r.status!==0){console.error(r.error?.message||r.stderr||'Unknown compiler error');stop('CANDIDATE_AVR_BUILD_FAILED '+fqbn);break;}
   pass++;console.log('CANDIDATE_AVR_COMPILE_PASS '+fqbn);
  }
  if(pass===2){console.log('P001_CANDIDATE_DUAL_BUILD_PASS=2/2');console.log('CANDIDATE_ONLY: not installed, not committed; safety/full-loop/HW acceptance still pending');}
 }finally{fs.rmSync(folder,{recursive:true,force:true});}
}
