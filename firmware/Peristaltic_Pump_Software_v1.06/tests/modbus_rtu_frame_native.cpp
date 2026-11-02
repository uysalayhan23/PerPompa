#include "../modbus_rtu_frame.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
using namespace perpompa_v106;
static int passed=0;
static void check(bool x){if(!x){fprintf(stderr,"FAIL at case %d\n",passed+1);__builtin_trap();}++passed;}
static void setCrc(uint8_t *p,uint8_t len){auto c=crc16(p,(uint8_t)(len-2));p[len-2]=(uint8_t)c;p[len-1]=(uint8_t)(c>>8);}
static bool send(Receiver &r,const uint8_t *p,uint8_t len,uint32_t initial,uint32_t delta,Frame &result){
  for(uint8_t i=0;i<len;i++)if(r.feed(p[i],initial+(uint32_t)i*delta,result))return true;
  return r.idle(initial+(uint32_t)(len-1)*delta+4100,result);
}
int main(){
  check(crc16((const uint8_t*)"123456789",9)==0x4B37);
  check(charGap1_5(9600)==1719);
  check(frameGap3_5(9600)==4011);
  check(charGap1_5(38400)==750 && frameGap3_5(38400)==1750);
  Receiver good(1,9600);check(good.configurationValid());
  Receiver bad0(0,9600),bad248(248,9600);check(!bad0.configurationValid());check(!bad248.configurationValid());
  Receiver invalidBaud(1,0);check(!invalidBaud.configurationValid());
  uint8_t read[]={1,3,0,0,0,2,0,0};setCrc(read,8);Frame out={};
  check(validCrc(read,8));check(send(good,read,8,10000,1050,out));check(out.len==8 && out.adu[1]==3);
  uint8_t corrupt[8];for(int i=0;i<8;++i)corrupt[i]=read[i];corrupt[4]^=0x40;
  check(!validCrc(corrupt,8));check(!send(good,corrupt,8,50000,1050,out));
  uint8_t wrong[8];for(int i=0;i<8;++i)wrong[i]=read[i];wrong[0]=2;setCrc(wrong,8);
  check(!send(good,wrong,8,90000,1050,out));
  uint8_t broadcast[8];for(int i=0;i<8;++i)broadcast[i]=read[i];broadcast[0]=0;setCrc(broadcast,8);
  check(!send(good,broadcast,8,130000,1050,out));
  check(!send(bad0,broadcast,8,150000,1050,out));
  good.reset();
  for(int i=0;i<4;++i)check(!good.feed(read[i],200000+i*1050,out));
  check(!good.feed(read[4],200000+4*1050+2500,out)); // gap above t1.5, below t3.5
  for(int i=5;i<8;++i)check(!good.feed(read[i],200000+i*1050+2500,out));
  check(!good.idle(220000,out)); // discarded entire malformed frame
  check(send(good,read,8,230000,1050,out));
  good.reset();
  for(int i=0;i<4;++i)check(!good.feed(read[i],260000+i*1050,out));
  check(!good.idle(266000,out)); // truncated
  good.reset();
  check(send(good,read,8,0xFFFFEE00UL,1050,out)); // micros wrap
  check(!validCrc(nullptr,0));
  check(!validCrc(read,3));
  good.reset();
  for(int i=0;i<65;++i)check(!good.feed((uint8_t)(i+1),300000+i*400,out));
  check(!good.idle(340000,out)); // oversized
  check(send(good,read,8,360000,1050,out));
  printf("P106_MODBUS_FRAME_NATIVE=%d/%d PASS\n",passed,passed);
}
