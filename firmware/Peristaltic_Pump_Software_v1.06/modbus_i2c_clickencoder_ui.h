#ifndef PERPOMPA_V106_MODBUS_I2C_CLICKENCODER_UI_H
#define PERPOMPA_V106_MODBUS_I2C_CLICKENCODER_UI_H
// v1.06: actual hd44780_I2Cexp and ClickEncoder API adapter around the proven
// pure Modbus Settings model. NOT a complete motor firmware or a flashable sketch.
// IMPORTANT: existing loop must read encoder->getButton()/read_encoder_step()
// exactly once, and route them EXCLUSIVELY here while the Modbus page owns UI.
// The 1ms ClickEncoder service/Timer1 interrupt must be retained by the caller.
// This adapter never starts/stops the motor or touches Serial/RS485 settings.
#include <stdint.h>
#include <string.h>
#include <ClickEncoder.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include "modbus_menu_lcd_bridge.h"
namespace perpompa_v106 {

// Propagate I2C write failure into local authorization. A partial displayed
// confirmation is NOT sufficient to authorize an EEPROM change.
class CheckedI2cLcd {
  hd44780_I2Cexp &lcd_;
  bool healthy_;
public:
  explicit CheckedI2cLcd(hd44780_I2Cexp &lcd):lcd_(lcd),healthy_(false){}
  bool begin(){
    healthy_=lcd_.begin(16,2)==0;
    return healthy_;
  }
  bool healthy() const {return healthy_;}
  void setCursor(uint8_t col,uint8_t row){
    if(!healthy_||lcd_.setCursor(col,row)!=0)healthy_=false;
  }
  void print(const char *text){
    if(!healthy_||!text)return;
    const size_t len=strlen(text);
    if(lcd_.print(text)!=len)healthy_=false;
  }
};

template<class Eeprom> class ModbusI2cClickUi {
  CheckedI2cLcd screen_;
  ModbusMenuLcdBridge<Eeprom,CheckedI2cLcd> menu_;
  bool holdLatch_;
  bool lcdReady_;
  // Reject any stale changes after local controls stop owning the menu.
  void failClosed(){
    // Cancel confirmation/edit first, then close the menu completely.
    for(uint8_t n=0;n<3 && menu_.visible();++n)(void)menu_.cancel();
    holdLatch_=false;
  }
public:
  ModbusI2cClickUi(Eeprom &eeprom,hd44780_I2Cexp &lcd)
    :screen_(lcd),menu_(eeprom,screen_),holdLatch_(false),lcdReady_(false){}
  bool begin(){
    lcdReady_=screen_.begin();
    if(!lcdReady_)failClosed();
    return lcdReady_;
  }
  // The caller enters only when its EXISTING main-menu cursor points at the
  // dedicated Modbus Settings item, and before handling that same click twice.
  bool open(const UiSafety &s){
    if(!lcdReady_||!screen_.healthy()||!s.safe())return false;
    holdLatch_=false;
    (void)menu_.open(s);
    if(!screen_.healthy())failClosed();
    return menu_.visible(); // a corrupt EEPROM shows a locked review page
  }
  bool ownsDisplay()const{return menu_.visible();}
  bool lcdHealthy()const{return lcdReady_&&screen_.healthy();}
  bool pendingUartChange()const{return menu_.hasPendingUartChange();}
  void acknowledgeUartApplied(){menu_.acknowledgeUartApplied();}
  Settings savedSettings()const{return menu_.activeSettings();}
  // Exactly one event and the already-filtered encoder detent per main loop.
  // Click, double-click, HOLD and release are distinct ClickEncoder events.
  UiResult process(ClickEncoder::Button event,int8_t detents,const UiSafety &s){
    if(!menu_.visible())return UiIgnored;
    if(!lcdHealthy()||!s.safe()){
      failClosed();return UiBlocked;
    }
    UiResult result=UiIgnored;
    if(detents!=0){
      result=menu_.turn(detents,s);
      if(!screen_.healthy()){failClosed();return UiBlocked;}
    }
    switch(event){
      case ClickEncoder::Clicked:
        result=menu_.click(s);break;
      case ClickEncoder::DoubleClicked:
        result=menu_.cancel();break;
      case ClickEncoder::Held:
        if(!holdLatch_){holdLatch_=true;result=menu_.hold(s);}
        break;
      case ClickEncoder::Released:
        holdLatch_=false;break;
      case ClickEncoder::Open:
      case ClickEncoder::Closed:
      case ClickEncoder::Pressed:
        break;
    }
    if(!screen_.healthy()){
      failClosed();return UiBlocked;
    }
    return result;
  }
};
} // namespace perpompa_v106
#endif
