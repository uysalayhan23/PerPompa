#include "p0-02-usb-parser.h"
#include <cstdio>
#include <cstdlib>
#include <cstdint>
using perpompa_usb::Parser;
using perpompa_usb::Event;
using namespace perpompa_usb;
static int pass=0;
static void require(bool okay,const char* name){
 if(!okay){std::fprintf(stderr,"P002_QUEUE_ORDER_FAIL: %s\n",name);std::exit(2);}++pass;
}
static Event feedText(Parser& p,const char* txt,uint32_t t){
 Event e={kNone,{0,0,0}};
 for(const char* cur=txt;*cur;++cur){const Event x=p.feed(static_cast<uint8_t>(*cur),t);if(x.kind!=kNone)e=x;}
 return e;
}
int main(){
 Parser old;
 feedText(old,"p1000 500",100);
 const Event incorrectlyStarted=old.tick(141); // old order: tick BEFORE reading queued '0'
 require(incorrectlyStarted.kind==kPump&&incorrectlyStarted.value[1]==500,"old tick-first emits truncated pump");
 const Event lateDigit=old.feed('0',141);
 require(lateDigit.kind==kReject,"late digit wrongly rejected after premature finish");
 Parser fixed;
 feedText(fixed,"p1000 500",100);
 require(fixed.feed('0',141).kind==kNone,"new order must consume queued digit");
 require(fixed.tick(141).kind==kNone,"no premature pump after queued digit");
 const Event final=fixed.tick(182);
 require(final.kind==kPump&&final.value[0]==1000&&final.value[1]==5000,"full delayed field accepted correctly");
 Parser oldStop;
 feedText(oldStop,"p1000 500",100);
 require(oldStop.tick(141).kind==kPump,"old tick-first wrongly emits pump before queued stop");
 Parser fixedStop;
 feedText(fixedStop,"p1000 500",100);
 require(fixedStop.feed('x',141).kind==kStop,"queued x STOP wins before quiet completion");
 require(fixedStop.tick(142).kind==kNone,"STOP prevents stale pump event");
 Parser ordinary;
 feedText(ordinary,"p1000 5000",100);
 require(ordinary.tick(139).kind==kNone,"39ms quiet cannot accept");
 require(ordinary.tick(140).kind==kPump,"40ms quiet accepts when queue truly empty");
 Parser incomplete;
 feedText(incomplete,"d10 1000 ",100);
 require(incomplete.tick(141).kind==kReject,"missing third field fails closed");
 Parser wrap;
 feedText(wrap,"w4200",UINT32_MAX-9U);
 require(wrap.tick(29).kind==kNone,"wrap elapsed still below 40ms");
 require(wrap.tick(30).kind==kWriteCalibration,"wrap elapsed 40ms completes");
 std::printf("P002_QUEUE_ORDER_NATIVE_FCPU_%lu=%d/%d PASS; OLD_ORDER_BUG_REPRODUCED; NEW_ORDER_PREVENTS_PREMATURE_START; AVR=NOT_RUN; HW=NOT_RUN\n",(unsigned long)F_CPU,pass,pass);
}
