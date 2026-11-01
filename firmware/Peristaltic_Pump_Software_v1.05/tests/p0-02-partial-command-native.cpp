#include "p0-02-usb-parser.h"
#include <cstdint>
#include <iostream>
using namespace perpompa_usb;
int main(){
  int n=0;
  auto check=[&](bool ok,const char* label){if(!ok){std::cerr<<"FAIL "<<label<<"\n";return false;}++n;return true;};
  Parser p;
  if(!check(p.feed('w',0).kind==kPending,"w pending"))return 1;
  if(!check(p.feed('4',0).kind==kNone,"w partial"))return 1;
  if(!check(p.tick(39).kind==kNone,"w pretimeout"))return 1;
  if(!check(p.feed('x',40).kind==kStop,"stop precedes w timeout"))return 1;
  if(!check(p.tick(80).kind==kNone,"no stale w write"))return 1;
  if(!check(p.feed('w',100).kind==kPending,"w restart"))return 1;
  if(!check(p.feed('4',100).kind==kNone,"w digit"))return 1;
  if(!check(p.tick(140).kind==kWriteCalibration,"w quiet completion persists"))return 1;
  if(!check(p.feed('p',200).kind==kPending,"p pending"))return 1;
  p.feed('1',200);p.feed('0',200);p.feed(' ',200);p.feed('2',200);
  if(!check(p.tick(240).kind==kPump,"p quiet completion starts"))return 1;
  if(!check(p.feed('d',300).kind==kPending,"d pending"))return 1;
  p.feed('1',300);p.feed(' ',300);p.feed('2',300);
  if(!check(p.tick(340).kind==kReject,"incomplete d rejects"))return 1;
  if(!check(p.feed('p',400).kind==kPending,"p restart"))return 1;
  p.feed('1',400);p.feed(' ',400);p.feed('2',400);
  if(!check(p.feed('x',440).kind==kStop,"queued stop cancels p"))return 1;
  if(!check(p.tick(440).kind==kNone,"no p after stop"))return 1;
  std::cout<<"P002_PARTIAL_FCPU_"<<F_CPU<<"="<<n<<"/"<<n<<" PASS; PARSER_ONLY; PHYSICAL_STOP=NOT_RUN; FRAMING=REVIEW; RELEASE_BLOCKED\n";
}
