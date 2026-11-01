#include "p0-02-usb-parser.h"
#include <cstdint>
#include <iostream>
using namespace perpompa_usb;
int main(){
 int n=0;
 auto ck=[&](bool b){if(!b){std::cerr<<"FAIL "<<(n+1)<<'\n';return false;}++n;return true;};
 Parser p;
 if(!ck(p.feed('w',0).kind==kPending && p.pending()))return 1;
 if(!ck(p.feed('4',0).kind==kNone))return 1;
 if(!ck(p.tick(39).kind==kNone && p.pending()))return 1;
 if(!ck(p.tick(40).kind==kWriteCalibration && !p.pending()))return 1;
 if(!ck(p.feed('p',100).kind==kPending))return 1;
 p.feed('1',100);p.feed('0',100);p.feed(' ',100);p.feed('2',100);
 if(!ck(p.tick(139).kind==kNone))return 1;
 if(!ck(p.feed('x',140).kind==kStop && !p.pending()))return 1;
 if(!ck(p.tick(200).kind==kNone))return 1;
 if(!ck(p.feed('d',300).kind==kPending))return 1;
 p.feed('1',300);p.feed(' ',300);p.feed('2',300);p.feed(' ',300);p.feed('3',300);
 if(!ck(p.tick(340).kind==kDose))return 1;
 if(!ck(p.feed('w',UINT32_MAX-10).kind==kPending))return 1;
 p.feed('9',UINT32_MAX-10);
 if(!ck(p.tick(28).kind==kNone))return 1;
 if(!ck(p.tick(29).kind==kWriteCalibration))return 1;
 std::cout<<"P002_TRANSITIONS_FCPU_"<<F_CPU<<"="<<n<<"/"<<n<<" PASS; FRAMING=REVIEW; PHYSICAL_STOP=NOT_RUN; RELEASE_BLOCKED\n";
}
