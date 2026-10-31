#!/usr/bin/env node
'use strict';
// Static/reference diagnostics only. This is NOT an AVR compile, motor test or native firmware acceptance.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const assert = require('node:assert/strict');
const root = path.resolve(__dirname, '..', '..', '..');
const active = fs.readFileSync(path.join(__dirname, '..', 'Peristaltic_Pump_Software_v1.05.ino'));
const archived = fs.readFileSync(path.join(root, 'firmware/archive/Peristaltic_Pump_Software_v1.04/Peristaltic_Pump_Software_v1.04.ino'));
const source = active.toString('utf8');
const expectedBaseline = '0d965c08947ba0b3170d22fbff806fc2e6f1e83a';
function gitBlobSha(b) {
  return crypto.createHash('sha1').update(Buffer.from('blob ' + b.length + '\0')).update(b).digest('hex');
}
let passed=0, failed=0;
function check(name, fn) {
  try { fn(); passed++; process.stdout.write('SOURCE_OR_MODEL_PASS ' + name + '\n'); }
  catch (e) { failed++; process.stderr.write('FAIL ' + name + ': ' + e.message + '\n'); }
}
check('v1.04 archive immutable blob', () => assert.equal(gitBlobSha(archived), expectedBaseline));
check('v1.05 source exists', () => assert.ok(source.includes('void loop()') && source.includes('void setup()')));
check('Dose/Pump/Cal. interfaces retained', () => {
  for (const term of ['boolean dose(', 'void pump(', 'CALIBR_ROTATIONS 30','CALIBR_STEPS','void exit_action_menu()']) assert.ok(source.includes(term), term);
});
check('Remote/RNG and motor pin contract retained', () => {
  for (const term of ['#define REMOTE_PIN 5','#define REMOTE_DEBOUNCE_MS 30','#define RNG_PIN A0','#define MOTOR_STEP_PIN 7','#define MOTOR_DIR_PIN 6','REMOTE_HOLD','REMOTE_PULSE']) assert.ok(source.includes(term), term);
});
check('Encoder/LCD contract retained', () => {
  for (const term of ['#define LCD_COLUMNS 16','#define LCD_ROWS 2','read_encoder_step()', 'void update_lcd()', 'lcd_shown']) assert.ok(source.includes(term), term);
});
const rotations=30, microsteps=200*4;
const halfPeriodUs=(rate,unit,cal,dec)=>{
  const mLPerRotation=(cal/rotations)/1000;
  const conv=unit===2 ? 1 : (unit===1 ? 1/mLPerRotation/1000 : 1/mLPerRotation);
  return 30e6/(microsteps*conv*rate/(10**dec));
};
const steps=(volume,unit,cal,dec)=>{
  const mLPerRotation=(cal/rotations)/1000;
  const conv=unit===2 ? 1 : (unit===1 ? 1/mLPerRotation/1000 : 1/mLPerRotation);
  return microsteps*conv*volume/(10**dec);
};
check('model 30 rotations gives 24000 microsteps',()=>assert.equal(rotations*microsteps,24000));
check('model normal dose: 1 mL at calibration 5.000 mL = 4800 steps',()=>assert.equal(steps(10,0,5000,1),4800));
check('model normal pump: 10.0 mL/min at calibration 5.000 mL = 625us',()=>assert.equal(halfPeriodUs(100,0,5000,1),625));
check('diagnostic: cal=0 makes dose nonfinite (KNOWN_FINDING)',()=>assert.ok(!Number.isFinite(steps(10,0,0,1))));
check('diagnostic: rate=0 makes delay nonfinite (KNOWN_FINDING)',()=>assert.ok(!Number.isFinite(halfPeriodUs(0,0,5000,1))));
check('diagnostic: low rate exceeds 16-bit positive range (KNOWN_FINDING)',()=>assert.ok(halfPeriodUs(1,0,5000,1)>32767));
check('diagnostic: large volume/small cal exceeds signed 32-bit steps (KNOWN_FINDING)',()=>assert.ok(steps(9999,0,1,1)>2147483647));
console.log('REFERENCE_DIAGNOSTICS passed='+passed+' failed='+failed+' | HOST_CPP=NOT_RUN AVR_COMPILE=NOT_RUN HW_BENCH=NOT_RUN HW_PUMP=NOT_RUN');
if(failed) process.exitCode=1;
