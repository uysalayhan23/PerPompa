#include "../modbus_menu_lcd_bridge.h"
#include <stdio.h>
#include <string.h>
using namespace perpompa_v106;
static int tests=0;
static void check(bool condition){if(!condition){fprintf(stderr,"FAIL case %d\n",tests+1);__builtin_trap();}++tests;}
struct Ee {
  unsigned char mem[512];unsigned writes;
  Ee():writes(0){memset(mem,255,sizeof(mem));}
  unsigned char read(unsigned p)const{return mem[p];}
  void update(unsigned p,unsigned char v){++writes;mem[p]=v;}
};
struct Lcd {
  char rows[2][17];int writes;unsigned line;
  Lcd():writes(0),line(0){memset(rows,0,sizeof(rows));}
  void setCursor(unsigned x,unsigned y){check(x==0 && y<2);line=y;}
  void print(const char* s){check(strlen(s)==16);strcpy(rows[line],s);++writes;}
};
int main(){
  Ee ee;Lcd lcd;ModbusMenuLcdBridge<Ee,Lcd> bridge(ee,lcd);
  UiSafety safe={true,true,true,true,true};
  check(!bridge.visible());
  check(!bridge.hasPendingUartChange());
  UiSafety unsafe=safe;unsafe.localAccess=false;
  check(!bridge.open(unsafe));check(ee.writes==0);
  unsafe=safe;unsafe.motorIdle=false;check(!bridge.open(unsafe));
  unsafe=safe;unsafe.remoteIdle=false;check(!bridge.open(unsafe));
  unsafe=safe;unsafe.serialIdle=false;check(!bridge.open(unsafe));
  unsafe=safe;unsafe.hardwareReady=false;check(!bridge.open(unsafe));
  check(bridge.open(safe));check(bridge.lastLoad()==kDefaults);
  check(strncmp(lcd.rows[0],"Baud Rate",9)==0);
  check(strstr(lcd.rows[1],"9600")!=0);
  check(ee.writes==0);
  check(bridge.click(safe)==UiRedrawn); // edit baud
  check(bridge.turn(1,safe)==UiRedrawn); // 19200
  check(strstr(lcd.rows[1],"19200")!=0);
  check(bridge.cancel()==UiRedrawn); // exit edit only
  check(bridge.cancel()==UiExited); // leave no writes
  check(ee.writes==0 && !bridge.visible());
  check(bridge.open(safe));
  check(strstr(lcd.rows[1],"9600")!=0); // unsaved change discarded
  check(bridge.click(safe)==UiRedrawn);
  check(bridge.turn(1,safe)==UiRedrawn);
  check(bridge.click(safe)==UiRedrawn); // finish editing
  for(int i=0;i<4;++i)check(bridge.turn(1,safe)==UiRedrawn); // Save field
  check(bridge.click(safe)==UiRedrawn); // confirm screen
  check(strncmp(lcd.rows[0],"SAVE MODBUS CFG?",16)==0);
  unsafe=safe;unsafe.motorIdle=false;
  check(bridge.hold(unsafe)==UiBlocked);
  check(!bridge.visible() && ee.writes==0);
  check(bridge.open(safe));
  check(strstr(lcd.rows[1],"9600")!=0);
  check(bridge.click(safe)==UiRedrawn);
  check(bridge.turn(1,safe)==UiRedrawn);
  check(bridge.click(safe)==UiRedrawn);
  for(int i=0;i<4;++i)bridge.turn(1,safe);
  bridge.click(safe);
  check(bridge.hold(safe)==UiSaved);
  check(!bridge.visible());
  check(ee.writes>0);
  check(bridge.hasPendingUartChange());
  Settings a=bridge.activeSettings();check(baudForIndex(a.baudIndex)==19200);
  // UART must not be reconfigured automatically inside this UI adapter.
  bridge.acknowledgeUartApplied();check(!bridge.hasPendingUartChange());
  for(unsigned i=0;i<192;++i)check(ee.mem[i]==255);
  for(unsigned i=240;i<512;++i)check(ee.mem[i]==255);
  // Boot with valid persisted settings displays the same value.
  Lcd lcd2;ModbusMenuLcdBridge<Ee,Lcd> recovered(ee,lcd2);
  check(recovered.open(safe));check(recovered.lastLoad()==kLoaded);
  check(strstr(lcd2.rows[1],"19200")!=0);
  check(recovered.cancel()==UiExited);
  // Data corruption must lock the operator view, never allow applying a guessed default.
  Ee corrupt;corrupt.mem[kEepromSlot0]='M';Lcd badLcd;
  ModbusMenuLcdBridge<Ee,Lcd> locked(corrupt,badLcd);
  check(!locked.open(safe));check(locked.visible());
  check(locked.lastLoad()==kCorrupt);
  check(strncmp(badLcd.rows[0],"MODBUS CFG LOCK",15)==0);
  check(locked.click(safe)==UiBlocked);
  check(locked.hold(safe)==UiBlocked);
  check(corrupt.writes==0);
  check(locked.cancel()==UiExited);
  // EEPROM generation conflict also blocks.
  Ee conflict=ee;const unsigned a0=kEepromSlot0;
  const unsigned b0=kEepromSlot1;
  // Clone first slot as second and change one setting without increasing generation.
  for(unsigned i=0;i<kRecordSize;++i)conflict.mem[b0+i]=conflict.mem[a0+i];
  conflict.mem[b0+5]=42;
  auto crc=settingsCrc16(conflict.mem+b0,12);
  conflict.mem[b0+12]=(uint8_t)crc;conflict.mem[b0+13]=(uint8_t)(crc>>8);
  Lcd conflictLcd;ModbusMenuLcdBridge<Ee,Lcd> ambiguous(conflict,conflictLcd);
  check(!ambiguous.open(safe));check(ambiguous.lastLoad()==kConflict);
  check(conflict.writes==ee.writes);
  printf("P106_MODBUS_LCD_BRIDGE_NATIVE=%d/%d PASS\n",tests,tests);
}
