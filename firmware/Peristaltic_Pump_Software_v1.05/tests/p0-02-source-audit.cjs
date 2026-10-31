#!/usr/bin/env node
'use strict';
// This gate records *open* P0-02 hazards; it does NOT claim time acceptance.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'..');
const ino=path.join(root,'Peristaltic_Pump_Software_v1.05.ino');
const b=fs.readFileSync(ino),sha=crypto.createHash('sha1')
 .update('blob '+b.length).update(Buffer.from([0])).update(b).digest('hex');
const expected='04185aaa799b0af0bfa663b8acc8f5013a515066';
function body(s,name){const k=s.indexOf(name);if(k<0||s.indexOf(name,k+name.length)!==-1)
 throw Error('AMBIGUOUS_SOURCE '+name);let d=0,i=s.indexOf('{',k);
 for(let j=i;j<s.length;j++){if(s[j]==='{')d++;else if(s[j]==='}'&&!--d)return s.slice(k,j+1)}
 throw Error('UNBALANCED '+name);}
try{
 if(sha!==expected)throw Error('SOURCE_BLOB_MISMATCH');
 const source=b.toString('utf8'),loop=body(source,'void loop()'),
 pump=body(source,'void pump(int _delay_us) {'),
 dose=body(source,'boolean dose(long _steps, int _delay_us, long & inc) {');
 const count=(s,x)=>s.split(x).length-1;
 if(count(loop,'Serial.parseInt()')!==6||source.includes('Serial.setTimeout('))
   throw Error('SERIAL_PARSE_PATH_CHANGED');
 if(count(pump,'delayMicroseconds(_delay_us)')!==2||
    count(dose,'delayMicroseconds(_delay_us)')!==2)
   throw Error('STEP_WAIT_PATH_CHANGED');
 if(loop.indexOf('Serial.parseInt()')>loop.lastIndexOf('handle_remote_input();'))
   throw Error('STOP_ORDER_CHANGED');
 if(!loop.includes('delay(700)'))throw Error('EEPROM_SAVE_WAIT_PATH_CHANGED');
 if(count(source,'menu[1].suffix=="uL"')!==2||
    count(source,'menu[3].suffix=="uL/min"')!==1)throw Error('SUFFIX_WARNING_PATH_CHANGED');
 if(!loop.includes('switch (b)')||!loop.includes('case ClickEncoder::Held:'))
   throw Error('ENCODER_SWITCH_CHANGED');
 console.log('P002_SOURCE_AUDIT=PASS');
 console.log('STEP_DELAY_HALF_PERIOD_US=4..16000; TWO_BLOCKING_HALVES_PER_STEP=2');
 console.log('SERIAL_PARSEINT_CALLS=6; DEFAULT_STREAM_TIMEOUT_MS=1000 (Arduino core reference)');
 console.log('REMOTE_STOP_CHECK_AFTER_ACTION=YES; REMOTE_DEBOUNCE_MS=30');
 console.log('EEPROM_SAVE_BLOCKING_DELAY_MS=700; PLUS_22_WRITE_OPERATIONS=SOURCE_PATH');
 console.log('STRING_POINTER_COMPARE_WARNINGS=3; SWITCH_UNHANDLED_ENUM_WARNINGS=1');
 console.log('P002_STOP_LATENCY=REVIEW; AVR_WORST_CASE_TIMING=NOT_RUN; RELEASE_BLOCKED');
}catch(e){console.error('P002_SOURCE_AUDIT=BLOCKED '+e.message);process.exitCode=2;}
