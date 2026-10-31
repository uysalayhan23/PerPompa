#include <cstdint>
#include <cstdio>
#include <cstring>
#include "hw-setup-runtime.h"
using namespace perpompa_hw;
struct FakeEEPROM {
  uint8_t b[128]; int reads=0;
  FakeEEPROM(){memset(b,0xff,sizeof b);}
  uint8_t read(int index){reads++;return b[index];}
  void update(int index,uint8_t value){b[index]=value;}
};
int main() {
 FakeEEPROM ep;
 const uint32_t cpu=F_CPU;
 Profile p={(uint8_t)(cpu==16000000UL?kProMini5V16MHz:kProMini3V3_8MHz),kA4988,kRequiredMicrosteps,true};
 uint8_t record[kRecordLength];
 if(!encodePending(p,1,record))return 10;
 for(int i=0;i<11;i++) ep.update(kSlot0Address+i,record[i]);
 ep.update(kSlot0Address+11,kCommitMarker);ep.update(kSetupArmAddress,kSetupArmed);
 SafeSetup<FakeEEPROM> safe(ep,cpu);safe.boot();
 int n=0;
 for(int i=0;i<kStartPathCount;i++){
  ep.reads=0;
  if(!safe.permit((StartPath)i,false)||ep.reads!=25)return 11;
  n++;
 }
 ep.update(kSlot0Address+9,ep.b[kSlot0Address+9]^1);
 ep.reads=0;
 if(safe.permit(kLocalPump,false)||ep.reads!=25)return 12;
 n++;
 ep.update(kSlot0Address+9,ep.b[kSlot0Address+9]^1);
 ep.update(kSetupArmAddress,kSetupLocked);ep.reads=0;
 if(safe.permit(kUsbPump,false)||ep.reads!=1)return 13;
 n++;
 ep.update(kSetupArmAddress,kSetupArmed);ep.reads=0;
 if(safe.permit(kUsbPump,true)||ep.reads!=0)return 14;
 n++;
 std::printf("RUNTIME_HOT_PATH_CPU_%lu=%d/%d PASS; ONE_PERMISSION=25_READS; CRC_TAMPER_DENIED; ARM_REVOKE_DENIED; STOP_DENIED; FULL_SKETCH_NOT_RUN; HW_NOT_RUN\n",(unsigned long)cpu,n,n);
}
