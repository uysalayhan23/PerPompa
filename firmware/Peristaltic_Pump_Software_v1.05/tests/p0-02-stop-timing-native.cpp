// Source-derived isolated native test for PerPompa firmware SHA
// fd7a78a0ec4aae33eb466dc1cdaae671fa3ca02f.
// Mock elapsed microseconds are NOT physical STOP latency.
#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#define MOTOR_STEP_PIN 7
#define LOW 0
#define HIGH 1
using boolean=bool;
struct Menu {int value=0,lim=20000;};
Menu menu[11];
int cal=0,menu_number_1=9, inChar=0;
long step_counter=0,steps=0,rate_uL_min=0,vol_uL=0,delay_us=0;
bool usb_start=false, motorAllowed=true, motorRunning=false;
int idleCount=0, writes=0; long long mockUs=0;
std::vector<int> writeAddresses;
std::vector<int> pinStates;
void digitalWrite(int,int state){pinStates.push_back(state);}
void set_rng_running(bool b){motorRunning=b;}
void delayMicroseconds(int value){ assert(value>=0); mockUs+=value; }
bool hwMotorAllowed(){return motorAllowed;}
void exit_action_menu(){}
void hwForceIdle(){digitalWrite(MOTOR_STEP_PIN,LOW);set_rng_running(false);usb_start=false;idleCount++;}
void eepromWriteInt(int adr,int){writes+=2;writeAddresses.push_back(adr);writeAddresses.push_back(adr+1);}
long delay_us_calc(long,int,int,int){return 16000;}
long steps_calc(long,int,int,int){return 1;}
namespace perpompa_usb {
enum Kind {kNone,kPending,kStop,kReject,kCalibrate,kPump,kDose,kWriteCalibration};
struct Event {Kind kind; long value[3];};
}
// Exact firmware event function from lines 573-612; external dependencies mocked.
void usbApplyEvent(const perpompa_usb::Event &e){
  switch(e.kind){
    case perpompa_usb::kNone: return;
    case perpompa_usb::kPending:
    case perpompa_usb::kStop:
    case perpompa_usb::kReject:
      hwForceIdle(); inChar=0; step_counter=0; return;
    case perpompa_usb::kCalibrate:
      hwForceIdle(); inChar='c'; step_counter=0; usb_start=true; return;
    case perpompa_usb::kPump: {
      if(e.value[0]<=0 || e.value[1]<0 || e.value[1]>20000) {
        hwForceIdle(); inChar=0; return;
      }
      int calibration=(int)(e.value[1] ? e.value[1] : menu[7].value);
      long d=delay_us_calc(e.value[0],1,calibration,0);
      if(d<4 || d>16000){hwForceIdle();inChar=0;return;}
      hwForceIdle();rate_uL_min=e.value[0];cal=calibration;delay_us=d;
      inChar='p'; step_counter=0;usb_start=true; return;
    }
    case perpompa_usb::kDose: {
      if(e.value[0]<=0 || e.value[1]<=0 || e.value[2]<0 || e.value[2]>20000){
        hwForceIdle();inChar=0;return;
      }
      int calibration=(int)(e.value[2] ? e.value[2] : menu[7].value);
      long stepCount=steps_calc(e.value[0],1,calibration,0);
      long d=delay_us_calc(e.value[1],1,calibration,0);
      if(stepCount<=0 || d<4 || d>16000){hwForceIdle();inChar=0;return;}
      hwForceIdle();vol_uL=e.value[0];rate_uL_min=e.value[1];cal=calibration;
      steps=stepCount;delay_us=d;inChar='d';step_counter=0;usb_start=true;return;
    }
    case perpompa_usb::kWriteCalibration:
      // Preserve legacy w persistence while rejecting invalid calibration.
      hwForceIdle();inChar=0;
      if(e.value[0]<=0 || e.value[0]>menu[7].lim)return;
      cal=(int)e.value[0];menu[7].value=cal;
      for(int i=0;i<11;i++)eepromWriteInt(i*2,menu[i].value);
      return;
  }
  hwForceIdle(); inChar=0;
}
boolean dose(long _steps, int _delay_us, long & inc) {
  if (!hwMotorAllowed()) {hwForceIdle();inc=0;if(menu_number_1==0)exit_action_menu();return true;}
  if (_steps <= 0 || _delay_us < 4 || _delay_us > 16000) {
    digitalWrite(MOTOR_STEP_PIN, LOW);
    set_rng_running(false);
    inc = 0;
    return true;
  }
  if (inc < _steps) {
    set_rng_running(true);
    digitalWrite(MOTOR_STEP_PIN, HIGH);
    delayMicroseconds(_delay_us);
    digitalWrite(MOTOR_STEP_PIN, LOW);
    delayMicroseconds(_delay_us);
    inc++;
    return false;
  }
  set_rng_running(false);
  inc=0;
  return true;
}
void pump(int _delay_us) {
  if (!hwMotorAllowed()) {hwForceIdle();if(menu_number_1==0)exit_action_menu();return;}
  if (_delay_us < 4 || _delay_us > 16000) {
    digitalWrite(MOTOR_STEP_PIN, LOW);
    set_rng_running(false);
    return;
  }
  set_rng_running(true);
  digitalWrite(MOTOR_STEP_PIN, HIGH);
  delayMicroseconds(_delay_us);
  digitalWrite(MOTOR_STEP_PIN, LOW);
  delayMicroseconds(_delay_us);
}
void reset(){cal=0;inChar=0;usb_start=false;motorAllowed=true;motorRunning=false;idleCount=0;writes=0;mockUs=0;step_counter=0;pinStates.clear();writeAddresses.clear();menu[7].lim=20000;}
int main(){
 int n=0;
 auto check=[&](bool ok){if(!ok){std::cerr<<"FAIL test="<<n+1<<" F_CPU="<<F_CPU<<"\n";return false;}n++;return true;};
 reset(); usb_start=true;usbApplyEvent({perpompa_usb::kWriteCalibration,{4200,0,0}});
 if(!check(!usb_start&&idleCount==1&&writes==22&&cal==4200&&mockUs==0))return 1;
 if(!check(writeAddresses.size()==22&&writeAddresses.front()==0&&writeAddresses.back()==21))return 1;
 reset();usbApplyEvent({perpompa_usb::kWriteCalibration,{0,0,0}});
 if(!check(writes==0&&idleCount==1))return 1;
 reset();usbApplyEvent({perpompa_usb::kWriteCalibration,{20001,0,0}});
 if(!check(writes==0&&idleCount==1))return 1;
 reset();pump(16000);if(!check(mockUs==32000&&pinStates.size()==2&&pinStates[0]==HIGH&&pinStates[1]==LOW))return 1;
 reset();pump(4);if(!check(mockUs==8))return 1;
 reset();pump(16001);if(!check(mockUs==0&&!motorRunning))return 1;
 reset();motorAllowed=false;pump(16000);if(!check(mockUs==0&&idleCount==1))return 1;
 reset();long inc=0;boolean done=dose(2,16000,inc);if(!check(!done&&inc==1&&mockUs==32000))return 1;
 reset();inc=0;done=dose(1,4,inc);if(!check(!done&&mockUs==8))return 1;
 reset();inc=0;done=dose(1,16001,inc);if(!check(done&&mockUs==0&&inc==0))return 1;
 reset();inc=0;motorAllowed=false;done=dose(1,16000,inc);if(!check(done&&mockUs==0&&idleCount==1))return 1;
 reset();usb_start=true;usbApplyEvent({perpompa_usb::kStop,{0,0,0}});if(!check(!usb_start&&idleCount==1&&writes==0))return 1;
 std::cout<<"P002_STOP_TIMING_NATIVE_FCPU_"<<F_CPU<<"="<<n<<"/"<<n<<" PASS; MOCK_ONLY; PHYSICAL_STOP=NOT_RUN; FRAMING=REVIEW; RELEASE_BLOCKED\n";
}
