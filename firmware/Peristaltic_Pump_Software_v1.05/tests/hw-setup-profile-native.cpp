#include "hw-setup-profile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
using namespace perpompa_hw;
static int checks = 0;
void verify(bool ok, const char* label) {
  if(!ok) { fprintf(stderr,"HW_SETUP_HOST_FAIL %s\n",label); exit(1); }
  ++checks;
}
void commit(uint8_t* record) { record[11] = kCommitMarker; }
int main() {
#if F_CPU == 16000000UL
 const uint32_t cpu = 16000000UL;
#elif F_CPU == 8000000UL
 const uint32_t cpu = 8000000UL;
#else
#error Unsupported F_CPU for HW profile acceptance tests
#endif
 verify(kSlot0Address > 21 && kSlot1Address >= kSlot0Address+kRecordLength, "legacy EEPROM isolation");
 Profile initial = suggestedProfile();
 verify(initial.board == kUnknown && initial.driver == kA4988 && !initial.confirmed, "unconfirmed A4988 suggestion");
 verify(!validSelection(initial), "unknown board not valid");
 verify(!mayEdit(false,true,true), "no editing while motor running");
 verify(!mayEdit(true,false,true), "no editing during remote activity");
 verify(!mayEdit(true,true,false), "no editing during USB activity");
 verify(mayEdit(true,true,true), "editing allowed while idle");
 const uint8_t boards[2] = {kProMini5V16MHz,kProMini3V3_8MHz};
 const uint8_t drivers[2] = {kA4988,kDRV8825};
 for(int board=0;board<2;board++)for(int driver=0;driver<2;driver++) {
   Profile p = {boards[board], drivers[driver], kRequiredMicrosteps, true};
   uint8_t stored[2][kRecordLength];memset(stored,0xFF,sizeof(stored));
   uint8_t pending[kRecordLength];
   verify(validSelection(p), "four combinations valid selections");
   verify(encodePending(p,4,pending), "pending encode valid selection");
   verify(pending[11]==0, "pending is not yet committed");
   memcpy(stored[0],pending,kRecordLength);
   verify(!mayStart(stored[0],stored[1],cpu,false,false), "incomplete record locked");
   commit(stored[0]);
   const bool cpuMatch=cpuMatches(p,cpu);
   verify(cpuMatch==(p.board==(cpu==16000000UL?kProMini5V16MHz:kProMini3V3_8MHz)), "board CPU match");
   verify(mayStart(stored[0],stored[1],cpu,false,false)==cpuMatch, "stored confirmed profile CPU-locked");
   verify(!mayStart(stored[0],stored[1],cpu,true,false), "stop blocks start");
   verify(!mayStart(stored[0],stored[1],cpu,false,true), "setup view blocks start");
   verify(!mayStart(stored[0],stored[1],12000000UL,false,false), "unknown CPU always blocked");
   verify(!mayConfirm(p,cpu,true,true,false,true), "remote active blocks confirm");
   verify(!mayConfirm(p,cpu,true,true,true,false), "USB active blocks confirm");
   verify(!mayConfirm(p,cpu,false,true,true,true), "physical confirmation required");
   verify(mayConfirm(p,cpu,true,true,true,true)==cpuMatch, "confirm requires matching CPU");
   Profile changed = changedSelection(p, p.board, driver==0?kDRV8825:kA4988);
   verify(!changed.confirmed && changed.driver!=p.driver, "driver switch invalidates confirmation");
   changed = changedSelection(p,board==0?kProMini3V3_8MHz:kProMini5V16MHz,p.driver);
   verify(!changed.confirmed && changed.board!=p.board, "board switch invalidates confirmation");
   changed = changedSelection(p,p.board,p.driver);
   verify(changed.confirmed, "unchanged choice retains prior confirmation");
 }
 Profile p = {cpu==16000000UL?kProMini5V16MHz:kProMini3V3_8MHz,kA4988,4,true};
 uint8_t rec[2][kRecordLength],pending[kRecordLength];
 memset(rec,0xff,sizeof(rec));
 verify(!mayStart(rec[0],rec[1],cpu,false,false), "blank EEPROM fail closed");
 verify(encodePending(p,254,pending), "encode gen254");
 memcpy(rec[0],pending,kRecordLength);commit(rec[0]);
 verify(mayStart(rec[0],rec[1],cpu,false,false), "valid first record");
 for(int fault=0;fault<kRecordLength;fault++) {
   uint8_t torn[2][kRecordLength];memcpy(torn,rec,sizeof(torn));
   verify(encodePending(p,255,pending), "prepare record gen255");
   torn[1][11]=0; // invalidation first
   for(int i=0;i<fault && i<11;i++)torn[1][i]=pending[i];
   verify(mayStart(torn[0],torn[1],cpu,false,false), "power loss keeps previous valid slot");
 }
 verify(encodePending(p,255,pending), "encode gen255");
 memcpy(rec[1],pending,kRecordLength);commit(rec[1]);
 Profile resolved={};uint8_t gen=0,idx=255;
 verify(loadNewest(rec[0],rec[1],&resolved,&gen,&idx)&&gen==255&&idx==1,"newer slot selected");
 verify(encodePending(p,0,pending), "wraparound gen0");
 memcpy(rec[0],pending,kRecordLength);commit(rec[0]);
 verify(loadNewest(rec[0],rec[1],&resolved,&gen,&idx)&&gen==0&&idx==0,"generation wraparound");
 rec[0][9] ^= 0x01;
 verify(loadNewest(rec[0],rec[1],&resolved,&gen,&idx)&&gen==255&&idx==1,"CRC tamper falls back to valid slot");
 rec[1][10] ^= 0x01;
 verify(!mayStart(rec[0],rec[1],cpu,false,false),"both slots corrupt block motor");
 memset(rec,0xff,sizeof(rec));
 p.confirmed=false;verify(encodePending(p,1,pending),"encode unconfirmed record");
 memcpy(rec[0],pending,kRecordLength);commit(rec[0]);
 verify(!mayStart(rec[0],rec[1],cpu,false,false),"unconfirmed stored record blocks motor");
 p.confirmed=true;verify(encodePending(p,1,pending),"encode confirmed");
 memcpy(rec[0],pending,kRecordLength);commit(rec[0]);
 rec[0][2]=2;verify(!mayStart(rec[0],rec[1],cpu,false,false),"unknown version blocked");
 rec[0][2]=1;rec[0][4]=kUnknown;verify(!mayStart(rec[0],rec[1],cpu,false,false),"invalid board blocked");
 rec[0][9]=crc8(rec[0],9);rec[0][10]=static_cast<uint8_t>(rec[0][9]^0xffU);
 verify(!mayStart(rec[0],rec[1],cpu,false,false),"invalid board with valid CRC blocked");
 rec[0][4]=p.board;rec[0][5]=kUnknown;verify(!mayStart(rec[0],rec[1],cpu,false,false),"invalid driver blocked");
 rec[0][9]=crc8(rec[0],9);rec[0][10]=static_cast<uint8_t>(rec[0][9]^0xffU);
 verify(!mayStart(rec[0],rec[1],cpu,false,false),"invalid driver with valid CRC blocked");
 rec[0][5]=p.driver;rec[0][6]=8;verify(!mayStart(rec[0],rec[1],cpu,false,false),"wrong microstepping blocked");
 rec[0][9]=crc8(rec[0],9);rec[0][10]=static_cast<uint8_t>(rec[0][9]^0xffU);
 verify(!mayStart(rec[0],rec[1],cpu,false,false),"wrong microstepping with valid CRC blocked");
 p.microsteps=8;verify(!encodePending(p,2,pending),"invalid microstepping cannot be encoded");
 p.microsteps=4;p.board=kUnknown;verify(!encodePending(p,2,pending),"unknown board cannot be encoded");
 p.board=kProMini5V16MHz;p.driver=kUnknown;verify(!encodePending(p,2,pending),"unknown driver cannot be encoded");
 p.driver=kA4988;
 // A newer valid but unconfirmed driver change must override an older confirmed record.
 p.board=cpu==16000000UL?kProMini5V16MHz:kProMini3V3_8MHz;
 p.confirmed=true;verify(encodePending(p,11,pending),"encode last confirmed driver");
 memcpy(rec[0],pending,kRecordLength);commit(rec[0]);
 p.driver=kDRV8825;p.confirmed=false;
 verify(encodePending(p,12,pending),"encode newer unconfirmed alternate driver");
 memcpy(rec[1],pending,kRecordLength);commit(rec[1]);
 verify(!mayStart(rec[0],rec[1],cpu,false,false),"newer unconfirmed record blocks older confirmed record");
 p.confirmed=true; p.driver=kA4988;
 // Equivocal generations reject instead of guessing.
 p.board=cpu==16000000UL?kProMini5V16MHz:kProMini3V3_8MHz;
 verify(encodePending(p,4,pending),"encode conflicting generation A");
 memcpy(rec[0],pending,kRecordLength);commit(rec[0]);
 p.driver=kDRV8825;verify(encodePending(p,4,pending),"encode conflicting generation B");
 memcpy(rec[1],pending,kRecordLength);commit(rec[1]);
 verify(!mayStart(rec[0],rec[1],cpu,false,false),"equal generation conflicting contents blocked");
 p.driver=kA4988;verify(encodePending(p,132,pending),"encode half-distance generation");
 memcpy(rec[1],pending,kRecordLength);commit(rec[1]);
 verify(!mayStart(rec[0],rec[1],cpu,false,false),"ambiguous 128 distance blocked");
 printf("HW_SETUP_NATIVE_FCPU_%lu=%d/%d PASS; INO_INTEGRATION=NOT_RUN; HW=NOT_RUN\n",(unsigned long)cpu,checks,checks);
 return 0;
}
