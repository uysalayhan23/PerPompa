#!/usr/bin/env node
'use strict';
// Temporary HW Setup 01D source; reduce redundant profile scans.
// Live STEP guard and remote-entry authorization remain mandatory.
// No active .ino writes; no hardware claim.
const {build:build01c}=require('./hw-setup-01c-candidate.cjs');
const fixes=[["  if (!hwMotorAllowed()) { hwForceIdle(); exit_action_menu(); break; }\n","  if (menu[6].value < 0 || menu[6].value > 2) { hwForceIdle(); exit_action_menu(); break; }\n"],["    if (!hwMotorAllowed()) { hwForceIdle(); break; }\n",""],["  if (!hwMotorAllowed()) {hwForceIdle();inc=0;return true;}","  if (!hwMotorAllowed()) {hwForceIdle();inc=0;if(menu_number_1==0)exit_action_menu();return true;}"],["  if (!hwMotorAllowed()) {hwForceIdle();return;}","  if (!hwMotorAllowed()) {hwForceIdle();if(menu_number_1==0)exit_action_menu();return;}"]];
function replaceExactlyOnce(s,from,to){
 const count=s.split(from).length-1;
 if(count!==1)throw Error('HW_01D_PATCH_ANCHOR_FAIL count='+count+' '+from.slice(0,70));
 return s.replace(from,to);
}
function build(){
 let s=build01c();
 for(const [a,b] of fixes)s=replaceExactlyOnce(s,a,b);
 if((s.match(/if \(!hwMotorAllowed\(\)\)/g)||[]).length!==2)
   throw Error('HW_01D_PULSE_GUARDS_CHANGED');
 if(!s.includes('!hwSetup.permit(static_cast<perpompa_hw::StartPath>'))
   throw Error('HW_01D_REMOTE_PREFLIGHT_MISSING');
 if(!s.includes('if(menu_number_1==0)exit_action_menu();'))
   throw Error('HW_01D_LOCAL_DENIAL_STOP_MISSING');
 return s;
}
module.exports={build,fixes};
if(require.main===module){
 try{
  const s=build();
  console.log('HW_SETUP_01D_TEMP_CANDIDATE_READY chars='+s.length+
              '; PULSE_GUARD=ON; ORIGINAL_INO_UNCHANGED; AVR=NOT_RUN; HW=NOT_RUN');
 }catch(e){console.error('HW_SETUP_01D=BLOCKED '+e.message);process.exitCode=2;}
}
