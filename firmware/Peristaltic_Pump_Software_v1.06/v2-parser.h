#pragma once
#include <stdint.h>
#include <stddef.h>
namespace p003v2 {
enum Kind {None,Pending,Reject,Stop,Calibrate,Pump,Dose,Write};
struct Event { Kind kind; uint32_t v[3]; };
class Parser {
 char buf[49]{}; uint8_t n=0; bool active=false; uint32_t last=0;
 static bool white(char c){return c=='\n'||c=='\r'||c==' '||c=='\t';}
 static uint16_t crc(const char* s,size_t len){uint16_t c=0xffff;for(size_t i=0;i<len;i++){c^=(uint8_t)s[i]<<8;for(int b=0;b<8;b++)c=(c&0x8000)?(uint16_t)((c<<1)^0x1021):(uint16_t)(c<<1);}return c;}
 static bool num(const char* a,const char* b,uint32_t& v){if(a==b||b-a>10)return false;v=0;for(auto p=a;p!=b;++p){if(*p<'0'||*p>'9'||v>(2147483647u-(uint32_t)(*p-'0'))/10)return false;v=v*10+(*p-'0');}return true;}
 Event parse(){Event e{Reject,{0,0,0}};if(n<9||buf[0]!='@'||buf[1]!='2'||buf[2]!='|')return e;
 // Strict canonical @2|p|rate|cal|HHHH\n ; CRC is CCITT-FALSE over bytes before final '|HHHH'.
 int pipe[7],np=0;for(int i=0;i<n;i++)if(buf[i]=='|'){if(np==7)return e;pipe[np++]=i;}
 if(np<2||pipe[0]!=2||pipe[1]!=4||pipe[np-1]!=n-5)return e;
 char cmd=buf[3];int expected=cmd=='p'?4:cmd=='d'?5:cmd=='w'?3:cmd=='c'?2:0;if(np!=expected)return e;
 uint16_t got=0;for(int i=n-4;i<n;i++){char c=buf[i];uint8_t d=c>='0'&&c<='9'?c-'0':c>='A'&&c<='F'?c-'A'+10:255;if(d==255)return e;got=(uint16_t)((got<<4)|d);}
 if(got!=crc(buf,(size_t)pipe[np-1]))return e;
 for(int i=2;i<np;i++) if(!num(buf+pipe[i-1]+1,buf+pipe[i],e.v[i-2]))return Event{Reject,{0,0,0}};
 e.kind=cmd=='p'?Pump:cmd=='d'?Dose:cmd=='c'?Calibrate:Write;return e;
 }
 public:
 static uint16_t checksum(const char* s,size_t n){return crc(s,n);}void reset(){active=false;n=0;}
 Event feed(char c,uint32_t now){if(c=='x'){reset();return {Stop,{0,0,0}};}if(!active){if(white(c))return {None,{0,0,0}};if(c=='c')return {Reject,{0,0,0}};if(c!='@')return {Reject,{0,0,0}};active=true;n=0;}last=now;if(c=='\n'){Event e=parse();reset();return e;}if(n>=48){reset();return {Reject,{0,0,0}};}buf[n++]=c;return {Pending,{0,0,0}};}
 Event tick(uint32_t now){if(active&&(uint32_t)(now-last)>=40){reset();return {Reject,{0,0,0}};}return {None,{0,0,0}};}
};}
