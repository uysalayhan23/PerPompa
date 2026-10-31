#include "hw-setup-runtime.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace perpompa_hw;
struct FakeEeprom {
  uint8_t bytes[128];int writes;int failAfter;
  FakeEeprom() : writes(0),failAfter(-1) {memset(bytes,255,sizeof(bytes));}
  uint8_t read(uint16_t addr) const {if(addr>=sizeof(bytes))abort();return bytes[addr];}
  void update(uint16_t addr,uint8_t v){
    if(addr>=sizeof(bytes))abort();
    if(failAfter>=0 && writes>=failAfter)return; // simulate torn power or write fault
    writes++;bytes[addr]=v;
  }
};
static int checks=0;
void check(bool ok,const char* label){if(!ok){std::fprintf(stderr,"HW_SETUP_RUNTIME_FAIL: %s\n",label);std::exit(1);}++checks;}
void assertLockedAll(SafeSetup<FakeEeprom>& rt,const char* context){
  for(unsigned i=0;i<kStartPathCount;i++)check(!rt.permit(static_cast<StartPath>(i),false),context);
}
void assertAllowedAll(SafeSetup<FakeEeprom>& rt,const char* context){
  for(unsigned i=0;i<kStartPathCount;i++)check(rt.permit(static_cast<StartPath>(i),false),context);
  for(unsigned i=0;i<kStartPathCount;i++)check(!rt.permit(static_cast<StartPath>(i),true),"stop blocks all start paths");
}
int main(){
#if F_CPU == 16000000UL
 const uint32_t CPU=16000000UL, OTHER=8000000UL;
 const uint8_t BOARD=kProMini5V16MHz;
#elif F_CPU == 8000000UL
 const uint32_t CPU=8000000UL, OTHER=16000000UL;
 const uint8_t BOARD=kProMini3V3_8MHz;
#else
#error Unsupported F_CPU
#endif
 FakeEeprom ep; SafeSetup<FakeEeprom> rt(ep,CPU);rt.boot();
 char t[17],b[17];rt.lcdRows(t,b);
 check(strlen(t)==16&&strlen(b)==16,"LCD 16x2 padded");
 check(strncmp(t,"SETUP REQUIRED",14)==0,"initial display not ready");
 check(rt.draft().driver==kA4988&&rt.draft().board==kUnknown,"suggested A4988 is not confirmed");
 assertLockedAll(rt,"blank EEPROM locks every motor start");
 check(!rt.enter(false,true,true),"motor running blocks opening setup");
 check(!rt.enter(true,false,true),"remote running blocks opening setup");
 check(!rt.enter(true,true,false),"USB running blocks opening setup");
 check(rt.enter(true,true,true),"idle permits setup entry");
 check(ep.read(kSetupArmAddress)==kSetupLocked,"entry writes nonvolatile inhibit byte");
 assertLockedAll(rt,"editing locks every start path");
 rt.lcdRows(t,b);check(strncmp(t,"HW Setup: Board",15)==0,"board UI view");
 check(!rt.chooseBoard(kUnknown),"unknown board refused");
 check(rt.chooseBoard(BOARD),"chosen board");
 check(rt.nextPage(),"driver page");
 rt.lcdRows(t,b);check(strncmp(t,"HW Setup: Drive",15)==0,"driver page text");
 check(!rt.chooseDriver(3),"unknown driver refused");
 check(rt.chooseDriver(kA4988),"A4988 user selects");
 check(rt.nextPage(),"confirm page");
 rt.lcdRows(t,b);check(strncmp(b,"Press to confirm",16)==0,"confirm UI view");
 check(!rt.confirmAndSave(false,true,true,true),"missing physical confirmation locks");
 check(!rt.confirmAndSave(true,false,true,true),"motor active locks save");
 check(!rt.confirmAndSave(true,true,false,true),"remote active locks save");
 check(!rt.confirmAndSave(true,true,true,false),"usb active locks save");
 check(rt.confirmAndSave(true,true,true,true),"A4988 saved and confirmed");
 check(ep.read(kSetupArmAddress)==kSetupArmed,"armed only after complete verified save");
 assertAllowedAll(rt,"four modes all local remote USB authorized after explicit confirmation");
 rt.lcdRows(t,b);check(strncmp(t,"HW SETUP READY",14)==0,"ready view");
 SafeSetup<FakeEeprom> reboot(ep,CPU);reboot.boot();
 assertAllowedAll(reboot,"boot restores confirmed correct profile");
 SafeSetup<FakeEeprom> wrongCpu(ep,OTHER);wrongCpu.boot();
 assertLockedAll(wrongCpu,"wrong CPU cannot start");
 check(rt.enter(true,true,true),"reenter setup");
 assertLockedAll(rt,"entering existing profile deauthorizes old record");
 check(rt.nextPage()&&rt.chooseDriver(kDRV8825),"DRV8825 selected");
 rt.lcdRows(t,b);check(strncmp(b,"DRV8825",7)==0,"DRV8825 visible");
 check(rt.nextPage(),"DRV confirm page");
 check(!rt.confirmAndSave(false,true,true,true),"DRV selection never confirms automatically");
 assertLockedAll(rt,"unconfirmed DRV switch locked");
 SafeSetup<FakeEeprom> crashMidEdit(ep,CPU);crashMidEdit.boot();
 assertLockedAll(crashMidEdit,"power loss during setup locks previous profile");
 check(rt.confirmAndSave(true,true,true,true),"DRV8825 explicitly saved");
 assertAllowedAll(rt,"DRV8825 after confirmed profile allowed");
 rt.lcdRows(t,b);
 check(rt.enter(true,true,true),"reenter for cancel test");rt.cancel();
 assertLockedAll(rt,"cancel does not reauthorize previous profile");
 SafeSetup<FakeEeprom> crashAfterCancel(ep,CPU);crashAfterCancel.boot();
 assertLockedAll(crashAfterCancel,"cancel persists inhibit");
 check(crashAfterCancel.enter(true,true,true),"rearm setup after cancel");
 check(crashAfterCancel.chooseBoard(BOARD),"reselect board");
 check(crashAfterCancel.nextPage()&&crashAfterCancel.nextPage(),"advance to confirm");
 check(crashAfterCancel.confirmAndSave(true,true,true,true),"recommission");
 assertAllowedAll(crashAfterCancel,"recommissioned");
 // Simulate a partially written new record after the inhibit marker became durable.
 for(int fault=0;fault<15;fault++){
   FakeEeprom copy=ep; SafeSetup<FakeEeprom> s(copy,CPU);s.boot();
   check(s.enter(true,true,true),"torn write test enters setup");
   check(s.chooseBoard(BOARD),"torn write board");
   check(s.nextPage()&&s.nextPage(),"torn write confirm screen");
   copy.failAfter=copy.writes+fault;
   const bool saved=s.confirmAndSave(true,true,true,true);
   if(!saved){
     assertLockedAll(s,"failed write cannot output motor pulses");
     SafeSetup<FakeEeprom> after(copy,CPU);after.boot();
     assertLockedAll(after,"power cut cannot reauthorize old settings");
   } else {check(s.permit(kLocalDose,false),"fully committed before fault may run");}
 }
 // Stronger corrupted but correctly checksummed profile rejection.
 ep.bytes[kSlot0Address+4]=kUnknown;
 ep.bytes[kSlot1Address+4]=kUnknown;
 ep.bytes[kSlot0Address+9]=crc8(ep.bytes+kSlot0Address,9);
 ep.bytes[kSlot0Address+10]=ep.bytes[kSlot0Address+9]^0xff;
 ep.bytes[kSlot1Address+9]=crc8(ep.bytes+kSlot1Address,9);
 ep.bytes[kSlot1Address+10]=ep.bytes[kSlot1Address+9]^0xff;
 SafeSetup<FakeEeprom> corrupted(ep,CPU);corrupted.boot();
 assertLockedAll(corrupted,"invalid board despite valid CRC blocks all paths");
 std::printf("HW_SETUP_RUNTIME_FCPU_%lu=%d/%d PASS; INO=NOT_WIRED; AVR=NOT_RUN; HW=NOT_RUN\n",(unsigned long)CPU,checks,checks);
}
