#include "../modbus_i2c_clickencoder_ui.h"
#include <stdio.h>
#include <string.h>
using namespace perpompa_v106;
static unsigned checks=0;
static void checkAt(bool ok,int line){if(!ok){fprintf(stderr,"FAILED case %u line %d\n",checks+1,line);__builtin_trap();}++checks;}
#define check(x) checkAt((x),__LINE__)
struct FakeEe {
  uint8_t mem[512];int writes,cuts;
  FakeEe():writes(0),cuts(-1){memset(mem,255,sizeof(mem));}
  uint8_t read(unsigned p)const{return mem[p];}
  void update(unsigned p,uint8_t v){if(cuts>=0&&writes>=cuts){++writes;return;}++writes;mem[p]=v;}
};
int main(){
  const UiSafety ok={true,true,true,true,true};
  FakeEe ee;hd44780_I2Cexp lcd;
  ModbusI2cClickUi<FakeEe> ui(ee,lcd);
  check(!ui.ownsDisplay());check(!ui.open(ok)); // no begin, no I2C
  check(ui.begin());check(ui.lcdHealthy());
  check(ui.open(ok));check(ui.ownsDisplay());
  check(strncmp(lcd.row[0],"Baud Rate",9)==0);
  check(strstr(lcd.row[1],"9600")!=0);
  check(ui.process(ClickEncoder::Clicked,0,ok)==UiRedrawn); // edit Baud
  check(ui.process(ClickEncoder::Open,1,ok)==UiRedrawn);
  check(strstr(lcd.row[1],"19200")!=0);
  check(ui.process(ClickEncoder::Clicked,0,ok)==UiRedrawn); // end edit
  for(int i=0;i<4;++i)check(ui.process(ClickEncoder::Open,1,ok)==UiRedrawn);
  check(ui.process(ClickEncoder::Clicked,0,ok)==UiRedrawn); // Save + Apply
  check(strncmp(lcd.row[1],"HOLD TO CONFIRM",15)==0);
  check(ui.process(ClickEncoder::Held,0,ok)==UiSaved);
  check(!ui.ownsDisplay());check(ui.pendingUartChange());
  check(ee.writes>0);
  check(baudForIndex(ui.savedSettings().baudIndex)==19200);
  // Repeated Held without Released must not write twice or perform a second save.
  int oldWrites=ee.writes;
  check(ui.process(ClickEncoder::Held,0,ok)==UiIgnored);check(ee.writes==oldWrites);
  ui.acknowledgeUartApplied();check(!ui.pendingUartChange());
  // Persisted settings display on reboot, no stale defaults.
  hd44780_I2Cexp lcd2;ModbusI2cClickUi<FakeEe> rebooted(ee,lcd2);
  check(rebooted.begin());check(rebooted.open(ok));
  check(strstr(lcd2.row[1],"19200")!=0);
  check(rebooted.process(ClickEncoder::DoubleClicked,0,ok)==UiExited);
  // Safety gate: no opening if any one of the 5 conditions fails.
  for(int k=0;k<5;++k){
    UiSafety blocked=ok;
    if(k==0)blocked.localAccess=false;
    if(k==1)blocked.motorIdle=false;
    if(k==2)blocked.remoteIdle=false;
    if(k==3)blocked.serialIdle=false;
    if(k==4)blocked.hardwareReady=false;
    check(!rebooted.open(blocked));
  }
  // Safety change while editing drops full menu, no partial commits.
  check(rebooted.open(ok));check(rebooted.process(ClickEncoder::Clicked,0,ok)==UiRedrawn);
  UiSafety running=ok;running.motorIdle=false;
  check(rebooted.process(ClickEncoder::Open,1,running)==UiBlocked);
  check(!rebooted.ownsDisplay());check(ee.writes==oldWrites);
  // LCD begin fails => cannot open editor, does not write EEPROM.
  hd44780_I2Cexp noScreen;noScreen.initCode=4;
  ModbusI2cClickUi<FakeEe> failed(ee,noScreen);
  check(!failed.begin());check(!failed.lcdHealthy());check(!failed.open(ok));check(ee.writes==oldWrites);
  // Runtime I2C write failure blocks any new encoder actions or saved settings.
  hd44780_I2Cexp lost;ModbusI2cClickUi<FakeEe> disconnected(ee,lost);
  check(disconnected.begin());check(disconnected.open(ok));
  lost.shortWritesAfter=(int)lost.writes;
  check(disconnected.process(ClickEncoder::Clicked,0,ok)==UiBlocked);
  check(!disconnected.ownsDisplay());check(!disconnected.lcdHealthy());check(ee.writes==oldWrites);
  // A cursor command failure also cancels the editor before confirmation.
  hd44780_I2Cexp cursorError;ModbusI2cClickUi<FakeEe> cursorUi(ee,cursorError);
  check(cursorUi.begin());check(cursorUi.open(ok));
  cursorError.cursorCode=3;
  check(cursorUi.process(ClickEncoder::Clicked,0,ok)==UiBlocked);
  check(!cursorUi.ownsDisplay());
  check(ee.writes==oldWrites);
  // Corrupt EEPROM opens locked status display, no write or forced defaults.
  FakeEe broken;broken.mem[kEepromSlot0]=0x22;
  hd44780_I2Cexp alert;ModbusI2cClickUi<FakeEe> denied(broken,alert);
  check(denied.begin());check(denied.open(ok));
  check(strncmp(alert.row[0],"MODBUS CFG LOCK",15)==0);
  check(denied.process(ClickEncoder::Held,0,ok)==UiBlocked);
  check(broken.writes==0);
  check(denied.process(ClickEncoder::DoubleClicked,0,ok)==UiExited);
  // Power interruption before commit means old value survives.
  FakeEe cut=ee;cut.writes=0;cut.cuts=4;
  hd44780_I2Cexp cutScreen;ModbusI2cClickUi<FakeEe> partial(cut,cutScreen);
  check(partial.begin());check(partial.open(ok));
  partial.process(ClickEncoder::Clicked,0,ok);
  partial.process(ClickEncoder::Open,1,ok);
  partial.process(ClickEncoder::Clicked,0,ok);
  for(int i=0;i<4;++i)partial.process(ClickEncoder::Open,1,ok);
  partial.process(ClickEncoder::Clicked,0,ok);
  check(partial.process(ClickEncoder::Held,0,ok)==UiBlocked);
  int cutWrites=cut.writes;
  check(partial.process(ClickEncoder::Held,0,ok)==UiIgnored);
  check(cut.writes==cutWrites); // Held must not retry a failed EEPROM update
  check(baudForIndex(partial.savedSettings().baudIndex)==19200);
  check(!partial.pendingUartChange());
  // Check touch of EEPROM only in v1.06 reserved slots during live and cut writes.
  for(unsigned i=0;i<192;++i)check(ee.mem[i]==255);
  for(unsigned i=240;i<512;++i)check(ee.mem[i]==255);
  check(sizeof(ModbusI2cClickUi<FakeEe>)>0);
  printf("P106_I2C_CLICKENCODER_HOST_NATIVE=%u/%u PASS\n",checks,checks);
  return 0;
}
