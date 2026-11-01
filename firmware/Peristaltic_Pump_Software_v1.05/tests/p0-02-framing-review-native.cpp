// P0-02 legacy quiet completion regression gate adapted for accepted P0-03 V2.
// This tests parser/framing behavior, NOT physical STOP or all firmware paths.
#include "p0-02-usb-parser.h"
#include <stdint.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
using namespace perpompa_usb;
static unsigned checks=0;
static void ok(bool value,const char* name){
 if(!value){std::fprintf(stderr,"P002_V2_FRAMING_FAIL: %s\n",name);std::exit(2);}++checks;
}
static std::string frame(const std::string& body){
 uint16_t crc=p003v2::Parser::checksum(body.data(),body.size());
 char cs[8];std::snprintf(cs,sizeof(cs),"|%04X\n",(unsigned)crc);
 return body+cs;
}
static Event feed(Parser &p,const std::string &s,uint32_t now){
 Event last={kNone,{0,0,0}};
 for(char c:s){Event e=p.feed(static_cast<uint8_t>(c),now);
  if(e.kind!=kNone)last=e;}
 return last;
}
int main(){
 Parser legacyP;
 ok(feed(legacyP,"p1000 5000\n",100).kind==kReject,"legacy p rejected");
 ok(legacyP.tick(140).kind==kNone,"legacy p cannot resurrect");
 Parser legacyD;
 ok(feed(legacyD,"d100 1000 5000\n",100).kind==kReject,"legacy d rejected");
 Parser legacyW;
 ok(feed(legacyW,"w4200\n",100).kind==kReject,"legacy w rejected");
 Parser legacyC;
 ok(legacyC.feed('c',100).kind==kReject,"CRC-less c rejected");

 Parser p;std::string pc=frame("@2|p|1000|5000");
 ok(feed(p,pc,200).kind==kPending,"CRC pump deferred until queue drain");
 ok(p.tick(201).kind==kPump,"valid pump committed on drained tick");
 ok(p.tick(202).kind==kNone,"pump not replayed");
 Parser d;std::string dc=frame("@2|d|100|1000|5000");
 ok(feed(d,dc,300).kind==kPending,"CRC dose deferred");
 Event de=d.tick(301);
 ok(de.kind==kDose&&de.value[0]==100&&de.value[2]==5000,"CRC dose values preserved");
 Parser w;std::string wc=frame("@2|w|4200");
 ok(feed(w,wc,400).kind==kPending,"CRC write deferred");
 Event we=w.tick(401);
 ok(we.kind==kWriteCalibration&&we.value[0]==4200,"CRC write value preserved");
 Parser c;std::string cc=frame("@2|c");
 ok(feed(c,cc,500).kind==kPending,"CRC calibration deferred");
 ok(c.tick(501).kind==kCalibrate,"CRC calibration accepted at drain");

 Parser tamper;std::string bad=wc;bad[5]='3';
 ok(feed(tamper,bad,600).kind==kReject,"modified field without new CRC rejected");
 ok(tamper.tick(641).kind==kNone,"rejected CRC cannot replay");
 Parser truncated;
 ok(feed(truncated,"@2|w|420",700).kind==kPending,"incomplete V2 pending");
 ok(truncated.tick(739).kind==kNone,"39ms V2 incomplete not committed");
 ok(truncated.tick(740).kind==kReject,"40ms V2 incomplete rejected");
 Parser stop;
 ok(feed(stop,pc,800).kind==kPending,"pump held before x");
 ok(stop.feed('x',801).kind==kStop,"x immediate STOP cancels held pump");
 ok(stop.tick(801).kind==kNone,"x prevents stale pump delivery");
 Parser extra;
 ok(feed(extra,wc,900).kind==kPending,"write held before trailing bytes");
 ok(extra.feed(' ',900).kind==kReject,"trailing data cancels held EEPROM write");
 ok(extra.tick(900).kind==kNone,"canceled write cannot commit");
 Parser overflowing;
 ok(feed(overflowing,frame("@2|w|4200"),1000).kind==kPending,"held write before overbudget");
 overflowing.reset(); // unchanged firmware's case 9 overflow branch
 ok(overflowing.tick(1000).kind==kNone,"case9 overflow reset cancels write");
 Parser badLength;
 std::string tooLong="@2|w|"+std::string(49,'9');
 ok(feed(badLength,tooLong,1100).kind==kReject,"48-byte limit fails closed");
 Parser wrap;
 ok(feed(wrap,"@2|w|4",UINT32_MAX-19U).kind==kPending,"wrap frame pending");
 ok(wrap.tick(19).kind==kNone,"wrap at 39ms no event");
 ok(wrap.tick(20).kind==kReject,"wrap at 40ms rejects");
 std::printf("P002_V2_FRAMING_FCPU_%lu=%u/31 PASS; LEGACY_QUIET_COMPLETION=REJECTED; HOST_ONLY; PHYSICAL_NOT_RUN; RELEASE_BLOCKED\n",(unsigned long)F_CPU,checks);
 if(checks!=31)return 2;
}
