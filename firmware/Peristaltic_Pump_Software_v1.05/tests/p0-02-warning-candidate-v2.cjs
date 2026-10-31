#!/usr/bin/env node
'use strict';
// SHA-locked temporary candidate, fixes only 3 pointer comparisons and 1 enum switch.
// Never writes the accepted .ino and never flashes a controller.
const fs=require('node:fs'),path=require('node:path'),crypto=require('node:crypto');
const root=path.resolve(__dirname,'..');
const accepted='5b3c5a29ce8ce5eee0941469738b7e088d60a1bb';
function blob(bytes){return crypto.createHash('sha1').update('blob '+bytes.length)
 .update(Buffer.from([0])).update(bytes).digest('hex');}
const replacements=[
 ['menu[1].suffix=="uL"','strcmp(menu[1].suffix,"uL")==0',2],
 ['menu[3].suffix=="uL/min"','strcmp(menu[3].suffix,"uL/min")==0',1],
 ['switch (b) {\n      case ClickEncoder::Clicked:',
  'switch (b) {\n      case ClickEncoder::Open:\n      case ClickEncoder::Pressed:\n        break;\n      case ClickEncoder::Clicked:',1]
];
function build(){
 const sourceBytes=fs.readFileSync(path.join(root,'Peristaltic_Pump_Software_v1.05.ino'));
 if(blob(sourceBytes)!==accepted)throw Error('P002_WARNING_BASE_SHA_CHANGED');
 let source=sourceBytes.toString('utf8');
 if(!source.includes('#include <string.h>'))throw Error('STRING_HEADER_MISSING');
 for(const [oldValue,newValue,count] of replacements){
   const found=source.split(oldValue).length-1;
   if(found!==count)throw Error('P002_WARNING_ANCHOR '+found+' expected '+count+' '+oldValue);
   source=source.split(oldValue).join(newValue);
 }
 if(source.includes('.suffix=="'))throw Error('UNFIXED_STRING_LITERAL_COMPARISON');
 return source;
}
module.exports={build,replacements,accepted};
if(require.main===module){
 try{const candidate=build();console.log('P002_WARNING_TEMP_READY chars='+candidate.length+
 '; SOURCE_SHA_LOCKED; ORIGINAL_INO_UNCHANGED; FULL_CPP=NOT_RUN; AVR=NOT_RUN');}
 catch(e){console.error('P002_WARNING_TEMP=BLOCKED '+e.message);process.exitCode=2;}
}
