#include "p0-02-usb-parser.h"
#include <stdio.h>
#include <stdlib.h>
using namespace perpompa_usb;
static int checks=0;
static void check(bool b,const char* name){if(!b){fprintf(stderr,"P002_PARSER_FAIL %s\n",name);exit(1);}++checks;}
static Event push(Parser &p,const char *s,uint32_t time){Event e={kNone,{0,0,0}};
  for(const char *c=s;*c;c++){Event next=p.feed(static_cast<uint8_t>(*c),time);if(next.kind!=kNone)e=next;time++;}return e;}
int main(){
 Parser p;Event e;
 e=p.feed('p',100);check(e.kind==kPending&&p.pending(),"p starts pending not running");
 e=push(p,"1000 5000",101);check(e.kind==kNone,"p no final delimiter waits");
 e=p.tick(150);check(e.kind==kPump&&e.value[0]==1000&&e.value[1]==5000,"legacy p without EOL");
 check(!p.pending(),"p reset after complete");
 e=push(p,"d10 1000 5000\n",200);check(e.kind==kDose&&e.value[0]==10&&e.value[1]==1000&&e.value[2]==5000,"d with newline");
 e=push(p,"w4200\r",300);check(e.kind==kWriteCalibration&&e.value[0]==4200,"w saves only complete");
 e=p.feed('c',400);check(e.kind==kCalibrate,"c immediate");
 e=p.feed('x',401);check(e.kind==kStop,"x immediate");
 e=push(p,"p1000 ",500);check(p.pending(),"partial p stays pending");
 e=p.tick(548);check(e.kind==kReject&&!p.pending(),"missing p field rejects");
 e=push(p,"d10 100 ",600);e=p.tick(651);check(e.kind==kReject,"missing d third field rejects");
 e=push(p,"w",700);e=p.tick(740);check(e.kind==kReject,"bare w rejects");
 e=push(p,"p2000 50",800);e=p.feed('x',900);check(e.kind==kStop&&!p.pending(),"x interrupts incomplete payload");
 e=push(p,"d99999999999",1000);check(e.kind==kReject,"overlong integer rejected");
 e=push(p,"p2147483648 ",1100);check(e.kind==kReject,"int32 overflow rejected");
 e=push(p,"p-1 20",1200);check(e.kind==kReject,"negative value rejected");
 e=push(p,"p1;2",1300);check(e.kind==kReject,"malformed separator rejected");
 e=push(p,"p12 garbage",1400);check(e.kind==kReject,"garbage rejected");
 e=push(p,"p1 2",1500);e=p.tick(1535);check(e.kind==kNone,"no early completion");
 e=p.tick(1545);check(e.kind==kPump&&e.value[0]==1&&e.value[1]==2,"quiet-time complete");
 e=push(p,"d  1   2    3  ",1600);check(e.kind==kDose,"whitespace compatibility");
 e=push(p,"z",1700);check(e.kind==kReject,"unknown command rejected");
 e=push(p,"w0 ",1800);check(e.kind==kWriteCalibration&&e.value[0]==0,"w0 parsed but caller must reject zero");
 e=push(p,"p1 2",1900);e=p.feed('x',1910);check(e.kind==kStop,"x can interrupt p before quiet period");
 e=push(p,"d1 2 3",2000);e=p.feed('x',2010);check(e.kind==kStop,"x can interrupt d before quiet period");
 e=push(p,"w5",2100);e=p.feed('x',2110);check(e.kind==kStop,"x can interrupt w");
 e=push(p,"p1 2",2200);e=p.tick(0xFFFFFFFEu);check(e.kind==kPump||e.kind==kReject,"timer unsigned large elapsed fail closed");
 e=p.feed('p',0xFFFFFFF0u);e=push(p,"10 20",0xFFFFFFF1u);e=p.tick(50u);check(e.kind==kPump&&e.value[0]==10&&e.value[1]==20,"unsigned millis wrap");
 e=p.feed('p',100);for(int i=0;i<48;i++)e=p.feed(' ',101+i);check(e.kind==kNone&&p.pending(),"bounded payload within max");
 e=p.feed(' ',200);check(e.kind==kReject&&!p.pending(),"payload overflow aborts");
 e=push(p,"p1 2 ",300);check(e.kind==kPump,"valid after overflow recovery");
 e=push(p,"p1 2 ",400);check(e.kind==kPump,"repeated legacy commands");
 e=push(p,"p1 2 x",500);check(e.kind==kStop,"STOP after completed command wins");
 e=push(p,"p1 2 3",600);check(e.kind==kReject,"extra number cannot start motor");
 e=push(p,"c",700);check(e.kind==kCalibrate,"cal after parser reset");
 // STOP is an immediate single-byte event even when a partial packet contains data.
 e=p.feed('p',800);e=p.feed('x',801);check(e.kind==kStop,"back-to-back p/x");
 printf("P002_USB_NONBLOCK_NATIVE_FCPU_%lu=%d/%d PASS; SOURCE_UNIT; AVR=NOT_RUN; HW=NOT_RUN\n",(unsigned long)F_CPU,checks,checks);
}
