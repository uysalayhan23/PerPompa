#ifndef PERPOMPA_P002_USB_PARSER_H
#define PERPOMPA_P002_USB_PARSER_H
// Isolated, bounded, nonblocking parser. No Arduino APIs, GPIO or EEPROM access.
// p(rate,cal), d(volume,rate,cal), w(cal), c, x. Space/tab/CR/LF separate numbers.
// Legacy packets can omit the final delimiter: a quiet gap closes the last token.
// A pending/invalid command should STOP the active motor in the caller.
#include <stdint.h>
namespace perpompa_usb {
enum EventKind : uint8_t { kNone, kPending, kStop, kReject, kPump, kDose, kCalibrate, kWriteCalibration };
struct Event {
  EventKind kind;
  int32_t value[3];
};
class Parser {
 public:
  static const uint16_t kQuietMs=40;
  static const uint8_t kMaxBytes=48;
  static const uint8_t kMaxDigits=10;
  Parser(){reset();}
  void reset(){ command_=0; required_=0; count_=0; digits_=0; bytes_=0; current_=0; lastMs_=0;
    for(uint8_t i=0;i<3;i++)value_[i]=0; }
  bool pending() const {return command_!=0;}
  Event feed(uint8_t ch, uint32_t nowMs) {
    if(ch=='x'){reset();return event(kStop);}
    if(command_==0){
      if(isWhite(ch))return event(kNone);
      if(ch=='c')return event(kCalibrate);
      if(ch=='p'||ch=='d'||ch=='w'){
        reset();command_=ch;required_=ch=='d'?3:ch=='p'?2:1;lastMs_=nowMs;
        return event(kPending);
      }
      return event(kReject);
    }
    if(++bytes_>kMaxBytes){reset();return event(kReject);}
    lastMs_=nowMs;
    if(ch>='0'&&ch<='9'){
      if(count_>=required_||digits_>=kMaxDigits){reset();return event(kReject);}
      const uint8_t digit=static_cast<uint8_t>(ch-'0');
      // Use a signed 32-bit range on both AVR and host. Never overflow first.
      if(current_>(INT32_MAX-digit)/10){reset();return event(kReject);}
      current_=current_*10+digit;
      ++digits_;return event(kNone);
    }
    if(isWhite(ch)){
      if(digits_!=0){
        if(count_>=3){reset();return event(kReject);}
        value_[count_++]=current_;current_=0;digits_=0;
        if(count_==required_)return complete();
      }
      return event(kNone);
    }
    reset();return event(kReject);
  }
  Event tick(uint32_t nowMs){
    if(!pending() || static_cast<uint32_t>(nowMs-lastMs_)<kQuietMs)return event(kNone);
    if(digits_!=0){
      if(count_>=3){reset();return event(kReject);}
      value_[count_++]=current_;
    }
    reset();return event(kReject);
  }
 private:
  uint8_t command_,required_,count_,digits_,bytes_;
  int32_t current_,value_[3];uint32_t lastMs_;
  static bool isWhite(uint8_t ch){return ch==' '||ch=='\t'||ch=='\r'||ch=='\n';}
  static Event event(EventKind kind){Event e={kind,{0,0,0}};return e;}
  Event complete(){
    Event e=event(command_=='p'?kPump:command_=='d'?kDose:kWriteCalibration);
    for(uint8_t i=0;i<3;i++)e.value[i]=value_[i];
    reset();return e;
  }
};
} // namespace perpompa_usb
#endif
