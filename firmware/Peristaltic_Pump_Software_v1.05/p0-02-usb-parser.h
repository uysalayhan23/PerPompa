#ifndef PERPOMPA_P002_USB_PARSER_H
#define PERPOMPA_P002_USB_PARSER_H
// TEMPORARY V2 adapter: deliver completed commands only at the queue-drained tick.
// Any trailing byte in the same UART batch cancels the pending command, while 'x'
// remains an immediate STOP. The production firmware's case 9 stays unchanged.
#include "v2-parser.h"
namespace perpompa_usb {
enum EventKind : uint8_t { kNone, kPending, kStop, kReject, kPump, kDose, kCalibrate, kWriteCalibration };
struct Event {EventKind kind;int32_t value[3];};
class Parser {
  p003v2::Parser impl_;
  Event held_;
  bool heldValid_;
  static Event cvt(p003v2::Event x) {
    Event o={kReject,{0,0,0}};
    switch(x.kind) {
      case p003v2::None: o.kind=kNone; break;
      case p003v2::Pending: o.kind=kPending; break;
      case p003v2::Reject: o.kind=kReject; break;
      case p003v2::Stop: o.kind=kStop; break;
      case p003v2::Calibrate: o.kind=kCalibrate; break;
      case p003v2::Pump: o.kind=kPump; break;
      case p003v2::Dose: o.kind=kDose; break;
      case p003v2::Write: o.kind=kWriteCalibration; break;
    }
    for(uint8_t i=0;i<3;i++)o.value[i]=(int32_t)x.v[i];
    return o;
  }
  static bool action(EventKind kind){
    return kind==kPump||kind==kDose||kind==kCalibrate||kind==kWriteCalibration;
  }
 public:
  Parser():held_{kNone,{0,0,0}},heldValid_(false){}
  void reset(){impl_.reset();heldValid_=false;}
  Event feed(uint8_t c,uint32_t t){
    if(c=='x'){reset();return {kStop,{0,0,0}};}
    // A completed frame is not committed until case 9 finishes examining its
    // currently queued bytes. Reject *any* additional byte in this batch.
    if(heldValid_){reset();return {kReject,{0,0,0}};}
    Event e=cvt(impl_.feed((char)c,t));
    if(action(e.kind)){
      held_=e;heldValid_=true;
      return {kPending,{0,0,0}};
    }
    if(e.kind==kStop||e.kind==kReject)heldValid_=false;
    return e;
  }
  Event tick(uint32_t t){
    // case 9 invokes tick only once the UART input queue is empty and the
    // overflow/drop branch did not fire. No EEPROM/STEP action is issued in feed.
    Event e=cvt(impl_.tick(t));
    if(e.kind==kReject||e.kind==kStop){heldValid_=false;return e;}
    if(heldValid_){heldValid_=false;return held_;}
    return e;
  }
};
}
#endif
