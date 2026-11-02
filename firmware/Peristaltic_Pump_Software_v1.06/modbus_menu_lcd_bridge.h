#ifndef PERPOMPA_V106_MODBUS_MENU_LCD_BRIDGE_H
#define PERPOMPA_V106_MODBUS_MENU_LCD_BRIDGE_H
// v1.06 isolated Modbus settings -> generic 16x2 LCD adapter.
// Arduino UART, actual I2C library and encoder event wiring are NOT integrated.
// Does not issue any motor command, modify motor GPIO or grant an START permit.
#include "modbus_settings.h"
namespace perpompa_v106 {
struct UiSafety {
  bool localAccess;
  bool motorIdle;
  bool remoteIdle;
  bool serialIdle;
  bool hardwareReady;
  bool safe() const {
    return localAccess && motorIdle && remoteIdle && serialIdle && hardwareReady;
  }
};
enum UiResult : uint8_t { UiIgnored=0, UiRedrawn=1, UiExited=2, UiSaved=3, UiBlocked=4 };
template<class Eeprom, class Lcd> class ModbusMenuLcdBridge {
  SettingsStore<Eeprom> store_;
  ModbusMenu menu_;
  Lcd &lcd_;
  LoadStatus lastLoad_;
  bool locked_;
  bool visible_;
  bool pendingUartChange_;
  void draw() {
    char top[17],bottom[17];
    if(locked_){
      copy(top,"MODBUS CFG LOCK");copy(bottom,"LOCAL REVIEW REQD");
    } else {
      menu_.rows(top,bottom);
    }
    lcd_.setCursor(0,0);lcd_.print(top);
    lcd_.setCursor(0,1);lcd_.print(bottom);
  }
  static void copy(char *dst,const char *src) {
    uint8_t i=0;for(;i<16&&src[i];++i)dst[i]=src[i];
    for(;i<16;++i)dst[i]=' ';
    dst[16]=0;
  }
  UiResult eventResult(ModbusMenu::Result x){
    if(x==ModbusMenu::Exit){visible_=false;return UiExited;}
    if(x==ModbusMenu::Redraw){draw();return UiRedrawn;}
    return UiIgnored;
  }
  bool interactive(const UiSafety &s){
    if(!visible_||locked_||!menu_.active())return false;
    if(!s.safe()){
      // Communication settings must not be adjusted while any control owner
      // is active; close the UI without committing a pending draft.
      menu_.cancel();visible_=false;return false;
    }
    return true;
  }
public:
  ModbusMenuLcdBridge(Eeprom &e,Lcd &l):store_(e),menu_(),lcd_(l),
    lastLoad_(kDefaults),locked_(false),visible_(false),pendingUartChange_(false){}
  bool open(const UiSafety &s){
    if(!s.safe())return false;
    Settings cfg=defaultSettings();
    lastLoad_=store_.load(cfg);
    locked_=lastLoad_==kConflict||lastLoad_==kCorrupt;
    visible_=true;
    if(!locked_ && !menu_.open(cfg,true)){visible_=false;return false;}
    draw();
    return !locked_;
  }
  UiResult turn(int8_t steps,const UiSafety&s){
    if(!interactive(s))return UiBlocked;
    return eventResult(menu_.turn(steps));
  }
  UiResult click(const UiSafety&s){
    if(!interactive(s))return UiBlocked;
    return eventResult(menu_.click());
  }
  UiResult hold(const UiSafety&s){
    if(!interactive(s))return UiBlocked;
    if(menu_.hold(s.motorIdle,s.remoteIdle,s.serialIdle)!=ModbusMenu::SaveRequested)return UiIgnored;
    const Settings previous=activeSettings();
    bool success=store_.save(menu_.draft(),true,s.motorIdle,s.remoteIdle,s.serialIdle);
    Settings readback={};
    if(success && (store_.load(readback)!=kLoaded || !sameSettings(readback,menu_.draft())))success=false;
    menu_.saveFinished(success);
    if(success){
      pendingUartChange_=!sameSettings(previous,readback);
      visible_=false;
      return UiSaved;
    }
    draw();return UiBlocked;
  }
  UiResult cancel(){
    if(!visible_)return UiIgnored;
    if(locked_){locked_=false;visible_=false;return UiExited;}
    return eventResult(menu_.cancel());
  }
  Settings activeSettings()const{
    Settings s=defaultSettings();
    store_.load(s);
    return s;
  }
  bool visible()const{return visible_;}
  bool hasPendingUartChange()const{return pendingUartChange_;}
  void acknowledgeUartApplied(){pendingUartChange_=false;}
  LoadStatus lastLoad()const{return lastLoad_;}
};
} // namespace perpompa_v106
#endif
