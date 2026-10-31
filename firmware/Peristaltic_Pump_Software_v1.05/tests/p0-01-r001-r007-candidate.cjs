#!/usr/bin/env node
'use strict';
// READ-ONLY P0-01+R-001/R-007 candidate; fails if baseline bytes changed.
// Does NOT alter repository files, flash hardware, or certify release.
const fs=require('node:fs'),path=require('node:path'),os=require('node:os'),
      cp=require('node:child_process'),crypto=require('node:crypto');
const folder=path.resolve(__dirname,'..');
const sourcePath=path.join(folder,'Peristaltic_Pump_Software_v1.05.ino');
const archive=path.resolve(folder,'..','archive','Peristaltic_Pump_Software_v1.04','Peristaltic_Pump_Software_v1.04.ino');
const baseline='0d965c08947ba0b3170d22fbff806fc2e6f1e83a';
function sha(file){const b=fs.readFileSync(file);return crypto.createHash('sha1').update(Buffer.from('blob '+b.length+'\0')).update(b).digest('hex');}
function fail(msg){console.error('P001_RISK_CANDIDATE_BLOCKED '+msg);process.exitCode=2;}
function replaceFunction(src,sig,body){const i=src.indexOf(sig);
 if(i<0||src.indexOf(sig,i+sig.length)>=0)throw Error('Ambiguous '+sig);
 const a=src.indexOf('{',i);let depth=0;
 for(let j=a;j<src.length;j++){if(src[j]==='{')depth++;if(src[j]==='}'&&!--depth)return src.slice(0,i)+body+src.slice(j+1);}
 throw Error('Unbalanced '+sig);
}
function replaceOnce(src,from,to){if(src.split(from).length!==2)throw Error('Non-unique patch anchor '+from.slice(0,50));return src.replace(from,to);}
const existing=fs.readFileSync(path.join(__dirname,'p0-01-candidate-dual-build.cjs'),'utf8');
function assignment(name){const m=existing.match(new RegExp('^ const '+name+'=(\\[[^\\r\\n]+\\]);$','m'));if(!m)throw Error('Baseline candidate missing '+name);return JSON.parse(m[1]);}
const riskRules=[["bool remote_started = false;","bool remote_started = false;\nbool remote_armed = false; // Require stable released input before first remote start."],[" remote_stable_state = remote_last_read;"," remote_stable_state = remote_last_read;\n remote_armed = (remote_stable_state == HIGH);"],["      } else {\n        menu[i].value = 0;\n      }","      } else {\n        // EEPROM Remote slot: fail to Pulse, not Hold on invalid/erased data.\n        menu[i].value = (i == 10) ? REMOTE_PULSE : 0;\n      }"],["  // A closed Hold contact is active immediately after power-up.\n  if (menu[10].value == REMOTE_HOLD && remote_stable_state == LOW){\n    remote_start();\n  }","  // Boot is fail-closed: even a saved Hold mode requires release and fresh closure."],["  remote_stable_state = reading;\n  if (menu[10].value == REMOTE_HOLD){","  remote_stable_state = reading;\n  // If the contact was LOW at boot, a debounced HIGH must be observed first.\n  if (!remote_armed){\n    if (reading == HIGH) remote_armed = true;\n    return;\n  }\n  if (menu[10].value == REMOTE_HOLD){"],["    } else if (inChar == 'x'){\n      usb_start=false;\n      set_rng_running(false);\n    }","    } else if (inChar == 'x'){\n      usb_start=false;\n      set_rng_running(false);\n      digitalWrite(MOTOR_STEP_PIN, LOW);\n    } else {\n      // Unknown input is not a mode: discard stale RUN and output state.\n      usb_start=false;\n      set_rng_running(false);\n      digitalWrite(MOTOR_STEP_PIN, LOW);\n    }"],["void exit_action_menu(){\n   set_rng_running(false);\n   in_action = false;","void exit_action_menu(){\n   set_rng_running(false);\n   // Leaving USB/Start actions must discard stale run state.\n   usb_start = false;\n   digitalWrite(MOTOR_STEP_PIN, LOW);\n   in_action = false;"]];
function makeCandidate(){let src=fs.readFileSync(sourcePath,'utf8');
 for(const [sig,body]of assignment('functions'))src=replaceFunction(src,sig,body);
 for(const [from,to]of assignment('anchors'))src=replaceOnce(src,from,to);
 for(const [from,to]of riskRules)src=replaceOnce(src,from,to);
 return src;
}
if(sha(sourcePath)!==baseline||sha(archive)!==baseline){fail('BASELINE_OR_ARCHIVE_SHA_MISMATCH');}
else {
 const candidate=makeCandidate();
 const temp=fs.mkdtempSync(path.join(os.tmpdir(),'perpompa-r001-r007-'));
 const target=path.join(temp,'Peristaltic_Pump_Software_v1.05');
 fs.mkdirSync(target);
 fs.writeFileSync(path.join(target,'Peristaltic_Pump_Software_v1.05.ino'),candidate);
 let passed=0;
 try {
   for(const cpu of ['16MHzatmega328','8MHzatmega328']){
     const fqbn='arduino:avr:pro:cpu='+cpu;
     const cli=process.env.ARDUINO_CLI||'arduino-cli';
     const run=cp.spawnSync(cli,['compile','--fqbn',fqbn,'--build-path',path.join(temp,'build-'+cpu),target],
       {encoding:'utf8',timeout:180000,maxBuffer:4194304});
     console.log('TARGET '+fqbn);
     if(run.stdout)console.log(run.stdout.trim());
     if(run.error||run.status!==0){fail('CANDIDATE_AVR_BUILD_FAIL '+fqbn+': '+(run.error?.message||run.stderr));break;}
     passed++;console.log('RISK_CANDIDATE_AVR_COMPILE_PASS '+fqbn);
   }
   if(passed===2)console.log('RISK_CANDIDATE_DUAL_AVR_PASS=2/2; FULL_LOOP_HOST_REQUIRES_SEPARATE_GATE; HW_NOT_RUN');
 } finally {fs.rmSync(temp,{recursive:true,force:true});}
}
