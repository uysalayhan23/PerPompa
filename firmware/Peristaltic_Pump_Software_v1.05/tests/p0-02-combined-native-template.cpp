#include "p0-02-usb-parser.h"
#include <cstdint>
#include <deque>
#include <string>
#include <iostream>
using namespace perpompa_usb;
#define CALIBR_STEPS 10
#define CALIBR_DELAY_US 100
#define MOTOR_STEP_PIN 5
#define LOW 0
#define HIGH 1
struct SerialMock {std::deque<char> data; int available(){return (int)data.size();} int read(){int c=(unsigned char)data.front();data.pop_front();return c;} void put(const std::string&s){for(char c:s)data.push_back(c);}} Serial;
Parser usbParser;
bool usbDropUntilEmpty=false,usb_start=false,rng_running=false;
char inChar=0;
long steps=10,step_counter=0,delay_us=100,rate_uL_min=0,vol_uL=0;
int cal=0,writeCount=0,stepCount=0,forceCount=0;
uint32_t currentMs=0;
struct MenuEntry {int value; int lim;};
MenuEntry menu[11];
uint32_t millis(){return currentMs;}
void digitalWrite(int pin,int level){if(pin==MOTOR_STEP_PIN && level==HIGH)stepCount++;}
void set_rng_running(bool b){rng_running=b;}
void hwForceIdle(){digitalWrite(MOTOR_STEP_PIN,LOW);set_rng_running(false);usb_start=false;forceCount++;}
long delay_us_calc(int32_t,int,int32_t,int){return 100;}
long steps_calc(int32_t,int,int32_t,int){return 5;}
void eepromWriteInt(int,int){writeCount+=2;}
void usbApplyEvent(const Event &e);
void pump(int){digitalWrite(MOTOR_STEP_PIN,HIGH);digitalWrite(MOTOR_STEP_PIN,LOW);}
bool dose(long,int,long&inc){digitalWrite(MOTOR_STEP_PIN,HIGH);digitalWrite(MOTOR_STEP_PIN,LOW);inc++;return false;}
/*APPLY_INSERT*/
void usbCycle(){switch(9){
/*CASE9_INSERT*/
}}
void reset(){Serial.data.clear();usbParser.reset();usbDropUntilEmpty=false;usb_start=false;rng_running=false;inChar=0;steps=10;step_counter=0;delay_us=100;rate_uL_min=vol_uL=0;cal=0;writeCount=stepCount=forceCount=0;currentMs=0;for(auto &e:menu){e.value=100;e.lim=20000;}}
int main(){int n=0;auto ck=[&](bool b,const char*name){if(!b){std::cerr<<"FAIL "<<name<<"\n";return false;}++n;return true;};
 reset();usb_start=true;inChar='p';Serial.put("w4");usbCycle();if(!ck(stepCount==0 && writeCount==0 && !usb_start,"partial w interrupts motor"))return 1;
 currentMs=39;usbCycle();if(!ck(writeCount==0 && stepCount==0,"w before quiet"))return 2;
 currentMs=40;usbCycle();if(!ck(writeCount==22 && stepCount==0,"w quiet writes 22"))return 3;
 reset();usb_start=true;inChar='p';Serial.put("w4");usbCycle();currentMs=40;Serial.put("x");usbCycle();if(!ck(writeCount==0 && stepCount==0 && !usb_start,"x cancels w at deadline"))return 4;
 currentMs=100;usbCycle();if(!ck(writeCount==0 && stepCount==0,"cancelled w cannot reappear"))return 5;
 reset();Serial.put("p100 100");usbCycle();if(!ck(stepCount==0 && writeCount==0,"partial p no step"))return 6;
 currentMs=39;usbCycle();if(!ck(stepCount==0,"p before timeout"))return 7;
 currentMs=40;usbCycle();if(!ck(stepCount==1 && writeCount==0,"p complete one step"))return 8;
 reset();Serial.put("p100 100");usbCycle();currentMs=40;Serial.put("x");usbCycle();if(!ck(stepCount==0 && writeCount==0 && !usb_start,"x cancels p"))return 9;
 reset();Serial.put("d100 100 100");usbCycle();if(!ck(stepCount==0,"partial d no step"))return 10;
 currentMs=40;usbCycle();if(!ck(stepCount==1 && writeCount==0,"d complete step"))return 11;
 reset();Serial.put("d100 100 100");usbCycle();currentMs=40;Serial.put("x");usbCycle();if(!ck(stepCount==0 && !usb_start,"x cancels d"))return 12;
 reset();Serial.put("d100 100");usbCycle();currentMs=40;usbCycle();if(!ck(stepCount==0 && writeCount==0 && !usb_start,"invalid d no side effects"))return 13;
 reset();Serial.put("w0 ");usbCycle();if(!ck(writeCount==0,"invalid calibration no write"))return 14;
 reset();Serial.put("w4 ");usbCycle();if(!ck(writeCount==22 && stepCount==0,"terminated w persist"))return 15;
 reset();usb_start=true;inChar='p';Serial.put("x");usbCycle();if(!ck(!usb_start && stepCount==0 && !rng_running,"active motor immediate stop"))return 16;
 std::cout<<"P002_COMBINED_FCPU_"<<F_CPU<<"="<<n<<"/"<<n<<" PASS; SOURCE_EXTRACTED; EEPROM_TIME=NOT_RUN; PHYSICAL_STOP=NOT_RUN; FRAMING=REVIEW; RELEASE_BLOCKED\n";
}
