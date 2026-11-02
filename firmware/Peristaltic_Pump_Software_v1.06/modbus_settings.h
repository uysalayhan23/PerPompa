#ifndef PERPOMPA_V106_MODBUS_SETTINGS_H
#define PERPOMPA_V106_MODBUS_SETTINGS_H
// Pure C++11 Modbus setup model and LCD menu. No motor commands or Arduino dependencies.
// NOT connected to the pump sketch yet; AVR and hardware acceptance are pending.
#include <stdint.h>
#include <stddef.h>
namespace perpompa_v106 {
struct Settings { uint8_t address, baudIndex, parity, stopBits; };
enum Parity : uint8_t { Even=0, Odd=1, None=2 };
static const uint8_t kBaudCount=7;
static const uint16_t kEepromSlot0=192, kEepromSlot1=224;
static const uint8_t kRecordSize=16, kCommit=0xA5;
inline uint32_t baudForIndex(uint8_t i) {
  static const uint32_t b[kBaudCount]={2400,4800,9600,19200,38400,57600,115200};
  return i<kBaudCount?b[i]:0;
}
inline Settings defaultSettings(){Settings s={1,2,Even,1};return s;}
inline bool validSettings(const Settings& s){
  return s.address>=1&&s.address<=247&&baudForIndex(s.baudIndex)!=0&&
         s.parity<=None&&(s.stopBits==1||s.stopBits==2);
}
inline uint8_t serialBits(const Settings& s){
  return (uint8_t)(1+8+(s.parity==None?0:1)+s.stopBits);
}
inline bool sameSettings(const Settings&a,const Settings&b){
  return a.address==b.address&&a.baudIndex==b.baudIndex&&a.parity==b.parity&&a.stopBits==b.stopBits;
}
inline uint16_t settingsCrc16(const uint8_t* p,uint8_t count){
  uint16_t crc=0xffff;
  for(uint8_t i=0;i<count;i++){
    crc^=(uint16_t)p[i]<<8;
    for(uint8_t bit=0;bit<8;bit++)crc=(crc&0x8000)?(uint16_t)((crc<<1)^0x1021):(uint16_t)(crc<<1);
  }
  return crc;
}
enum LoadStatus : uint8_t { kDefaults=0, kLoaded=1, kCorrupt=2, kConflict=3 };
template<class Eeprom> class SettingsStore {
  Eeprom &ee_;
  struct Record {Settings config;uint8_t generation;};
  bool erased(uint16_t address)const{
    for(uint8_t i=0;i<kRecordSize;i++)if(ee_.read(address+i)!=0xff)return false;
    return true;
  }
  bool decode(uint16_t address,Record &r)const{
    uint8_t data[kRecordSize];
    for(uint8_t i=0;i<kRecordSize;i++)data[i]=ee_.read(address+i);
    if(data[0]!='M'||data[1]!='B'||data[2]!=1||data[3]!=kRecordSize||data[15]!=kCommit||
       data[9]!=0||data[10]!=0||data[11]!=0||data[14]!=(uint8_t)(data[4]^0xff))return false;
    const uint16_t crc=settingsCrc16(data,12);
    if(data[12]!=(uint8_t)crc||data[13]!=(uint8_t)(crc>>8))return false;
    r.generation=data[4];r.config={data[5],data[6],data[7],data[8]};
    return validSettings(r.config);
  }
  void encode(const Settings&s,uint8_t generation,uint8_t b[kRecordSize])const{
    b[0]='M';b[1]='B';b[2]=1;b[3]=kRecordSize;b[4]=generation;
    b[5]=s.address;b[6]=s.baudIndex;b[7]=s.parity;b[8]=s.stopBits;
    b[9]=b[10]=b[11]=0;
    const uint16_t crc=settingsCrc16(b,12);
    b[12]=(uint8_t)crc;b[13]=(uint8_t)(crc>>8);
    b[14]=(uint8_t)(generation^0xff);b[15]=kCommit;
  }
  LoadStatus selected(Record &r,uint8_t &index)const{
    Record a={},b={};bool av=decode(kEepromSlot0,a),bv=decode(kEepromSlot1,b);
    if(!av&&!bv)return erased(kEepromSlot0)&&erased(kEepromSlot1)?kDefaults:kCorrupt;
    if(av&&bv){
      const uint8_t delta=(uint8_t)(b.generation-a.generation);
      if(delta==0){if(!sameSettings(a.config,b.config))return kConflict;}
      else if(delta==1){r=b;index=1;return kLoaded;}
      else if(delta!=255)return kConflict;
    }
    r=av?a:b;index=av?0:1;return kLoaded;
  }
public:
  explicit SettingsStore(Eeprom& ee):ee_(ee){}
  LoadStatus load(Settings &out)const{
    Record r={};uint8_t i=0;LoadStatus result=selected(r,i);
    if(result==kLoaded)out=r.config;
    else out=defaultSettings();
    return result;
  }
  bool save(const Settings&s,bool physicalConfirmation,bool motorIdle,bool remoteIdle,bool usbIdle){
    if(!physicalConfirmation||!motorIdle||!remoteIdle||!usbIdle||!validSettings(s))return false;
    Record old={};uint8_t index=0;LoadStatus status=selected(old,index);
    if(status==kConflict)return false;
    if(status==kLoaded&&sameSettings(old.config,s))return true;
    const uint16_t dest=(status==kLoaded&&index==0)?kEepromSlot1:kEepromSlot0;
    const uint8_t gen=status==kLoaded?(uint8_t)(old.generation+1):0;
    uint8_t b[kRecordSize];encode(s,gen,b);
    ee_.update(dest+15,0);
    if(ee_.read(dest+15)!=0)return false;
    for(uint8_t i=0;i<15;i++){
      ee_.update(dest+i,b[i]);
      if(ee_.read(dest+i)!=b[i])return false;
    }
    ee_.update(dest+15,kCommit);
    if(ee_.read(dest+15)!=kCommit)return false;
    Record verify={};uint8_t chosen=0;
    return selected(verify,chosen)==kLoaded&&chosen==(dest==kEepromSlot0?0:1)&&
           verify.generation==gen&&sameSettings(verify.config,s);
  }
};
class ModbusMenu {
public:
  enum Field : uint8_t { Baud=0, Node=1, ParityField=2, Stop=3, Save=4, Back=5 };
  enum Result : uint8_t { NoAction=0, Redraw=1, Exit=2, SaveRequested=3 };
private:
  Settings draft_;
  uint8_t selected_;
  bool active_,editing_,confirm_;
  static void put(char out[17],const char* in){
    uint8_t i=0;for(;i<16&&in[i];i++)out[i]=in[i];
    for(;i<16;i++)out[i]=' ';
    out[16]=0;
  }
  static void number(char out[17],unsigned long val){
    char reversed[11];uint8_t n=0;
    do{reversed[n++]=(char)('0'+val%10);val/=10;}while(val&&n<10);
    uint8_t i=0;while(n)out[i++]=reversed[--n];out[i]=0;
  }
public:
  ModbusMenu():draft_(defaultSettings()),selected_(Baud),active_(false),editing_(false),confirm_(false){}
  bool open(const Settings &current,bool motorIdle){
    if(!validSettings(current)||!motorIdle)return false;
    draft_=current;selected_=Baud;editing_=confirm_=false;active_=true;return true;
  }
  bool active()const{return active_;}
  bool editing()const{return editing_;}
  bool confirming()const{return confirm_;}
  Field field()const{return (Field)selected_;}
  const Settings& draft()const{return draft_;}
  Result turn(int8_t steps){
    if(!active_||confirm_||!steps)return NoAction;
    int delta=steps>0?1:-1;
    if(!editing_){ selected_=(uint8_t)((selected_+6+delta)%6);return Redraw; }
    switch(selected_){
      case Baud: draft_.baudIndex=(uint8_t)((draft_.baudIndex+kBaudCount+delta)%kBaudCount);break;
      case Node: {int next=(int)draft_.address+delta;draft_.address=(uint8_t)(next<1?247:next>247?1:next);break;}
      case ParityField: draft_.parity=(uint8_t)((draft_.parity+3+delta)%3);break;
      case Stop: draft_.stopBits=(uint8_t)(draft_.stopBits==1?2:1);break;
      default:return NoAction;
    }
    return Redraw;
  }
  Result click(){
    if(!active_)return NoAction;
    if(confirm_){confirm_=false;return Redraw;}
    if(editing_){editing_=false;return Redraw;}
    if(selected_==Back){active_=false;return Exit;}
    if(selected_==Save){confirm_=true;return Redraw;}
    editing_=true;return Redraw;
  }
  Result hold(bool motorIdle,bool remoteIdle,bool usbIdle){
    if(!active_||!confirm_||!motorIdle||!remoteIdle||!usbIdle)return NoAction;
    return validSettings(draft_)?SaveRequested:NoAction;
  }
  void saveFinished(bool succeeded){
    confirm_=false;
    if(succeeded)active_=false;
  }
  Result cancel(){
    if(!active_)return NoAction;
    if(confirm_)confirm_=false;
    else if(editing_)editing_=false;
    else {active_=false;return Exit;}
    return Redraw;
  }
  void rows(char top[17],char bottom[17])const{
    if(!active_){put(top,"Modbus Settings");put(bottom,"ENTER TO CONFIG");return;}
    if(confirm_){put(top,"SAVE MODBUS CFG?");put(bottom,"HOLD TO CONFIRM");return;}
    const char* labels[]={"Baud Rate","Node Address","Parity","Stop Bits","Save + Apply","Back"};
    put(top,labels[selected_]);
    char val[17]={};
    if(selected_==Baud)number(val,baudForIndex(draft_.baudIndex));
    else if(selected_==Node)number(val,draft_.address);
    else if(selected_==ParityField){const char* p[]={"EVEN","ODD","NONE"};put(val,p[draft_.parity]);}
    else if(selected_==Stop)number(val,draft_.stopBits);
    else if(selected_==Save)put(val,"CLICK TO REVIEW");
    else put(val,"CLICK TO EXIT");
    char display[17]={};display[0]=editing_?'*':'>';
    for(uint8_t i=0;i<15&&val[i];i++)display[i+1]=val[i];
    put(bottom,display);
  }
};
} // namespace perpompa_v106
#endif
