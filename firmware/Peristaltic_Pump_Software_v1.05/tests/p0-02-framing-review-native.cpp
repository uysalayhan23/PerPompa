#include "p0-02-usb-parser.h"
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
using namespace perpompa_usb;
static unsigned checks=0;
static void ok(bool value,const char* name){
 if(!value){std::fprintf(stderr,"P002_FRAMING_FAIL: %s\n",name);std::exit(2);}++checks;
}
static Event feed(Parser &p,const char *s,uint32_t now){
 Event last={kNone,{0,0,0}};
 for(const char* c=s;*c;c++){
  Event e=p.feed(static_cast<uint8_t>(*c),now);
  if(e.kind!=kNone)last=e;
 }
 return last;
}
int main(){
 Parser pump;
 ok(feed(pump,"p1000 500",100).kind==kPending,"p prefix recognized pending");
 ok(pump.tick(139).kind==kNone,"39ms no premature pump");
 Event truncated=pump.tick(140);
 ok(truncated.kind==kPump,"40ms truncated but numeric-valid p accepted");
 ok(truncated.value[0]==1000,"truncated p rate");
 ok(truncated.value[1]==500,"truncated p cal 500 instead of intended 5000");
 Parser intended;
 ok(feed(intended,"p1000 5000",100).kind==kPending,"intended packet pending");
 Event full=intended.tick(140);
 ok(full.kind==kPump,"full undelimited p accepted");
 ok(full.value[1]==5000,"full p cal expected 5000");
 Parser dose;
 ok(feed(dose,"d10 1000 500",200).kind==kPending,"d prefix pending");
 Event partialDose=dose.tick(240);
 ok(partialDose.kind==kDose,"partial third d field also accepted");
 ok(partialDose.value[2]==500,"partial d calibration changed");
 Parser write;
 ok(feed(write,"w420",300).kind==kPending,"w prefix pending");
 Event partialWrite=write.tick(340);
 ok(partialWrite.kind==kWriteCalibration,"partial w valid write event");
 ok(partialWrite.value[0]==420,"partial w value not intended 4200");
 Parser delimited;
 ok(feed(delimited,"w4200\n",400).kind==kWriteCalibration,"newline terminated w unambiguous");
 ok(delimited.tick(500).kind==kNone,"no duplicate after terminated w");
 Parser stop;
 feed(stop,"p1000 500",500);
 ok(stop.feed('x',540).kind==kStop,"STOP before quiet tick wins when processed first");
 ok(stop.tick(540).kind==kNone,"stop cannot emit stale p");
 Parser incomplete;
 feed(incomplete,"p1000 ",600);
 ok(incomplete.tick(640).kind==kReject,"truly missing field rejected");
 Parser invalid;
 ok(feed(invalid,"w2147483648",700).kind==kReject,"overflow rejected");
 Parser cal;
 ok(cal.feed('c',800).kind==kCalibrate,"calibration command preserved");
 Parser wrap;
 feed(wrap,"w4200",UINT32_MAX-19U);
 ok(wrap.tick(19).kind==kNone,"millis wrap 39ms pending");
 ok(wrap.tick(20).kind==kWriteCalibration,"millis wrap 40ms completes");
 std::printf("P002_FRAMING_SOURCE_FCPU_%lu=%u/%u PASS; TRUNCATED_VALID_LAST_FIELD_RISK=REPRODUCED; PHYSICAL_STOP=NOT_RUN; RELEASE_BLOCKED\n",
   (unsigned long)F_CPU,checks,checks);
}
