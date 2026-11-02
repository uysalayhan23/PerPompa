#include "../modbus_settings.h"
#include <stdio.h>
#include <string.h>
using namespace perpompa_v106;
static unsigned checks=0;
static void c(bool value){if(!value){fprintf(stderr,"FAIL case %u\n",checks+1);__builtin_trap();}++checks;}
struct FakeEe {
  uint8_t mem[512];int cuts,writes;
  FakeEe():cuts(-1),writes(0){memset(mem,0xff,sizeof(mem));}
  uint8_t read(unsigned p)const{return mem[p];}
  void update(unsigned p,uint8_t v){if(cuts>=0&&writes++>=cuts)return;mem[p]=v;}
};
int main(){
  Settings d=defaultSettings();
  c(d.address==1);c(baudForIndex(d.baudIndex)==9600);c(d.parity==Even);c(d.stopBits==1);c(serialBits(d)==11);
  for(int i=0;i<7;i++)c(baudForIndex((uint8_t)i)>0);
  c(baudForIndex(7)==0);c(validSettings(d));
  Settings invalid=d;invalid.address=0;c(!validSettings(invalid));invalid.address=248;c(!validSettings(invalid));
  invalid=d;invalid.baudIndex=8;c(!validSettings(invalid));invalid=d;invalid.parity=3;c(!validSettings(invalid));
  invalid=d;invalid.stopBits=0;c(!validSettings(invalid));invalid.stopBits=3;c(!validSettings(invalid));
  invalid=d;invalid.parity=None;invalid.stopBits=2;c(serialBits(invalid)==11);
  invalid.stopBits=1;c(serialBits(invalid)==10);
  FakeEe ee;SettingsStore<FakeEe> store(ee);Settings out={};
  c(store.load(out)==kDefaults);c(sameSettings(out,d));
  c(!store.save(d,false,true,true,true));c(!store.save(d,true,false,true,true));
  c(!store.save(d,true,true,false,true));c(!store.save(d,true,true,true,false));
  c(store.load(out)==kDefaults);
  c(store.save(d,true,true,true,true));c(store.load(out)==kLoaded);c(sameSettings(out,d));
  c(ee.mem[kEepromSlot0+15]==kCommit);c(ee.mem[kEepromSlot1+15]==0xff);
  int writes=ee.writes;c(store.save(d,true,true,true,true));c(ee.writes==writes);
  Settings changed=d;changed.address=42;changed.baudIndex=3;changed.parity=Odd;changed.stopBits=2;
  c(store.save(changed,true,true,true,true));c(store.load(out)==kLoaded);c(sameSettings(out,changed));
  c(ee.mem[kEepromSlot1+15]==kCommit);
  Settings again=changed;again.address=247;c(store.save(again,true,true,true,true));c(store.load(out)==kLoaded);c(sameSettings(out,again));
  for(int cut=0;cut<17;cut++){
    FakeEe cutEe=ee;cutEe.cuts=cut;cutEe.writes=0;
    SettingsStore<FakeEe> s(cutEe);Settings next=again;next.address=63;
    bool saved=s.save(next,true,true,true,true);
    Settings recovered={};LoadStatus state=s.load(recovered);
    c(state==kLoaded);c(sameSettings(recovered,again)||sameSettings(recovered,next));
    if(saved)c(sameSettings(recovered,next));
  }
  FakeEe erased;erased.mem[kEepromSlot0+0]='M';SettingsStore<FakeEe> corrupted(erased);
  c(corrupted.load(out)==kCorrupt);c(sameSettings(out,d));
  c(corrupted.save(d,true,true,true,true));c(corrupted.load(out)==kLoaded);
  FakeEe damaged=ee;damaged.mem[kEepromSlot0+5]^=0x80;SettingsStore<FakeEe> backup(damaged);
  c(backup.load(out)==kLoaded);c(sameSettings(out,changed));
  FakeEe conflict=ee; // transform generation in both records preserving checksums
  conflict.mem[kEepromSlot0+4]=100;conflict.mem[kEepromSlot0+14]=(uint8_t)(100^0xff);
  auto crc=settingsCrc16(conflict.mem+kEepromSlot0,12);
  conflict.mem[kEepromSlot0+12]=(uint8_t)crc;conflict.mem[kEepromSlot0+13]=(uint8_t)(crc>>8);
  SettingsStore<FakeEe> impossible(conflict);
  c(impossible.load(out)==kConflict);c(!impossible.save(d,true,true,true,true));
  ModbusMenu menu;char top[17],bottom[17];menu.rows(top,bottom);
  c(strncmp(top,"Modbus Settings",15)==0);c(!menu.open(d,false));c(menu.open(d,true));
  menu.rows(top,bottom);c(strncmp(top,"Baud Rate",9)==0);c(strstr(bottom,"9600")!=0);c(strlen(top)==16&&strlen(bottom)==16);
  c(menu.field()==ModbusMenu::Baud);c(menu.click()==ModbusMenu::Redraw);c(menu.editing());
  menu.turn(1);c(baudForIndex(menu.draft().baudIndex)==19200);c(menu.click()==ModbusMenu::Redraw);c(!menu.editing());
  menu.turn(1);c(menu.field()==ModbusMenu::Node);c(menu.click()==ModbusMenu::Redraw);
  menu.turn(-1);c(menu.draft().address==247);menu.turn(1);c(menu.draft().address==1);
  menu.turn(1);c(menu.draft().address==2);c(menu.click()==ModbusMenu::Redraw);
  menu.turn(1);c(menu.field()==ModbusMenu::ParityField);menu.click();menu.turn(1);c(menu.draft().parity==Odd);menu.click();
  menu.turn(1);c(menu.field()==ModbusMenu::Stop);menu.click();menu.turn(1);c(menu.draft().stopBits==2);menu.click();
  menu.turn(1);c(menu.field()==ModbusMenu::Save);c(menu.click()==ModbusMenu::Redraw);c(menu.confirming());
  menu.rows(top,bottom);c(strncmp(top,"SAVE MODBUS CFG?",16)==0);c(strncmp(bottom,"HOLD TO CONFIRM",15)==0);
  c(menu.hold(false,true,true)==ModbusMenu::NoAction);c(menu.hold(true,false,true)==ModbusMenu::NoAction);
  c(menu.hold(true,true,false)==ModbusMenu::NoAction);
  c(menu.hold(true,true,true)==ModbusMenu::SaveRequested);
  c(store.save(menu.draft(),true,true,true,true));menu.saveFinished(true);c(!menu.active());
  c(store.load(out)==kLoaded);c(sameSettings(out,menu.draft()));
  c(menu.open(out,true));menu.turn(-1);c(menu.field()==ModbusMenu::Back);c(menu.click()==ModbusMenu::Exit);
  c(!menu.active());c(menu.open(d,true));c(menu.cancel()==ModbusMenu::Exit);
  printf("P106_MODBUS_MENU_SETTINGS_NATIVE=%u/%u PASS\n",checks,checks);
}
