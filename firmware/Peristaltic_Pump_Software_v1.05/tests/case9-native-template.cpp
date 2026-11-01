#include "p0-02-usb-parser.h"
#include <cassert>
#include <cstdint>
#include <deque>
#include <string>
#include <iostream>
using namespace perpompa_usb;
#define CALIBR_STEPS 10
#define CALIBR_DELAY_US 100
struct FakeSerial {std::deque<char> data; int available(){return (int)data.size();} int read(){int x=(unsigned char)data.front();data.pop_front();return x;} void put(const std::string& s){for(char c:s)data.push_back(c);}} Serial;
Parser usbParser;
bool usbDropUntilEmpty=false,usb_start=false,rng=false;
char inChar=0;
long steps=10,step_counter=0;int delay_us=100;
int forceIdleCount=0,stepCount=0,writeCount=0,stopCount=0;
uint32_t now=0;
uint32_t millis(){return now;}
void set_rng_running(bool b){rng=b;}
void hwForceIdle(){usb_start=false;rng=false;forceIdleCount++;}
void usbApplyEvent(const Event&e){
 if(e.kind==kStop)stopCount++;
 if(e.kind==kPending||e.kind==kStop||e.kind==kReject){hwForceIdle();inChar=0;step_counter=0;return;}
 if(e.kind==kPump){inChar='p';delay_us=100;usb_start=true;}
 if(e.kind==kDose){inChar='d';steps=10;delay_us=100;usb_start=true;}
 if(e.kind==kCalibrate){inChar='c';usb_start=true;}
 if(e.kind==kWriteCalibration){hwForceIdle();writeCount+=22;}
}
void pump(int){stepCount++;}
bool dose(long,int,long&inc){stepCount++;inc++;return false;}
void usbCycle(){switch(9){
/*CASE9_INSERT*/
}}
void reset(){Serial.data.clear();usbParser.reset();usbDropUntilEmpty=false;usb_start=false;rng=false;inChar=0;steps=10;step_counter=0;delay_us=100;forceIdleCount=stepCount=writeCount=stopCount=0;now=0;}
int main(){int n=0;auto ok=[&](bool cond){if(!cond)return false; ++n;return true;};
 reset();usb_start=true;inChar='p';Serial.put("x");usbCycle();if(!ok(stopCount==1&&stepCount==0&&!usb_start))return 1;
 reset();usb_start=true;inChar='p';Serial.put(std::string(33,'x'));usbCycle();if(!ok(usbDropUntilEmpty&&stepCount==0&&!usb_start))return 2;usbCycle();if(!ok(!usbDropUntilEmpty))return 3;
 reset();Serial.put(std::string(32,'x'));usbCycle();if(!ok(!usbDropUntilEmpty&&stopCount==32))return 4;
 reset();Serial.put("p1000 500");usbCycle();now=39;usbCycle();if(!ok(!usb_start))return 5;now=40;usbCycle();if(!ok(usb_start&&stepCount==1))return 6;
 reset();Serial.put("w420");usbCycle();now=40;usbCycle();if(!ok(writeCount==22&&!usb_start))return 7;
 reset();Serial.put("w420");usbCycle();Serial.put("x");now=40;usbCycle();if(!ok(writeCount==0&&stopCount==1))return 8;
 reset();Serial.put("p1000 500");usbCycle();Serial.put("0");now=40;usbCycle();if(!ok(!usb_start))return 9;now=80;usbCycle();if(!ok(usb_start))return 10;
 reset();Serial.put("p1000 500");usbCycle();Serial.put("x");now=40;usbCycle();if(!ok(stepCount==0&&!usb_start))return 11;
 reset();Serial.put("w420");usbCycle();now=39;usbCycle();if(!ok(writeCount==0))return 12;
 reset();Serial.put("w4200 ");usbCycle();if(!ok(writeCount==22))return 13;
 std::cout<<"P002_CASE9_NATIVE_FCPU_"<<F_CPU<<"="<<n<<"/"<<n<<" PASS; SOURCE_EXTRACTED; PHYSICAL_STOP=NOT_RUN; FRAMING=REVIEW; RELEASE_BLOCKED\n";
}