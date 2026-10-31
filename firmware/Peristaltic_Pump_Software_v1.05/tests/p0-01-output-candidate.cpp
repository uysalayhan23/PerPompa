// P0-01 isolated candidate. NOT production firmware; no physical motor is driven.
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#define STEP_MODE 4
#define STEPS_PER_FULL_ROT 200
#define CALIBR_ROTATIONS 30
#define CALIBR_DECIMALS 3
#define MICROSEC_PER_SEC 1000000
const int CALIBR_DECIMAL_CORR=1000;
static constexpr double AVR_LONG_MAX=2147483647.0;
static constexpr double AVR_DELAY_SAFE_MAX_US=16383.0;
long steps_calc_safe(long volume,int unit_mode,int calibr,int decimals){
 if(volume<=0||unit_mode<0||unit_mode>2||decimals<0||decimals>3)return 0;
 if(unit_mode!=2&&calibr<=0)return 0;
 double factor=std::pow(10.0,decimals);
 double per_rot=(double(calibr)/CALIBR_ROTATIONS)/CALIBR_DECIMAL_CORR;
 double conv=unit_mode==2?1.0:(unit_mode==1?1.0/per_rot/1000.0:1.0/per_rot);
 double candidate=STEPS_PER_FULL_ROT*STEP_MODE*conv*volume/factor;
 if(!(candidate>=1.0&&candidate<=AVR_LONG_MAX))return 0;
 return static_cast<long>(candidate);
}
long delay_us_calc_safe(long rate,int unit_mode,int calibr,int decimals){
 if(rate<=0||unit_mode<0||unit_mode>2||decimals<0||decimals>3)return 0;
 if(unit_mode!=2&&calibr<=0)return 0;
 double factor=std::pow(10.0,decimals);
 double per_rot=(double(calibr)/CALIBR_ROTATIONS)/CALIBR_DECIMAL_CORR;
 double conv=unit_mode==2?1.0:(unit_mode==1?1.0/per_rot/1000.0:1.0/per_rot);
 double candidate=(1/(STEPS_PER_FULL_ROT*STEP_MODE*conv*rate/factor))*60*MICROSEC_PER_SEC/2;
 if(!(candidate>=2.0&&candidate<=AVR_DELAY_SAFE_MAX_US))return 0;
 return static_cast<long>(candidate);
}
#define MOTOR_STEP_PIN 7
#define HIGH 1
#define LOW 0
int highs=0,lows=0,delays=0;
bool rng_running=false;
void digitalWrite(int pin,int level){if(pin==MOTOR_STEP_PIN){if(level==HIGH)highs++;else lows++;}}
void delayMicroseconds(unsigned int){delays++;}
void set_rng_running(bool running){rng_running=running;}
bool safe_dose(long count,int delay_us,long &progress){
 if(count<=0||delay_us<=0){set_rng_running(false);progress=0;return true;}
 if(progress<count){
  set_rng_running(true);digitalWrite(MOTOR_STEP_PIN,HIGH);delayMicroseconds(delay_us);
  digitalWrite(MOTOR_STEP_PIN,LOW);delayMicroseconds(delay_us);progress++;return false;
 }
 set_rng_running(false);progress=0;return true;
}
void safe_pump(int delay_us){
 if(delay_us<=0){set_rng_running(false);return;}
 set_rng_running(true);digitalWrite(MOTOR_STEP_PIN,HIGH);delayMicroseconds(delay_us);
 digitalWrite(MOTOR_STEP_PIN,LOW);delayMicroseconds(delay_us);
}
void reset(){highs=lows=delays=0;rng_running=false;}
int main(){
 int passed=0;auto test=[&](bool cond,const char* name){
  if(!cond){std::fprintf(stderr,"FAIL %s\n",name);std::exit(1);}passed++;
 };
 long progress=0;
 reset();safe_pump(625);test(highs==1&&lows==1&&rng_running&&delays==2,"normal pump");
 reset();safe_pump(0);test(highs==0&&lows==0&&!rng_running&&delays==0,"zero-delay pump");
 reset();safe_pump(-5);test(highs==0&&lows==0&&!rng_running,"negative delay pump");
 reset();safe_pump(625);safe_pump(0);test(highs==1&&lows==1&&!rng_running,"invalid after valid");
 reset();progress=0;
 test(!safe_dose(2,625,progress)&&highs==1&&rng_running&&progress==1,"dose first");
 test(!safe_dose(2,625,progress)&&highs==2&&progress==2,"dose second");
 test(safe_dose(2,625,progress)&&highs==2&&!rng_running&&progress==0,"dose complete");
 reset();progress=0;test(safe_dose(1,0,progress)&&highs==0&&!rng_running,"dose zero-delay");
 reset();progress=2;test(safe_dose(2,-1,progress)&&highs==0&&progress==0&&!rng_running,"dose negative-delay");
 reset();progress=0;test(safe_dose(0,625,progress)&&highs==0&&!rng_running,"dose zero target");
 reset();progress=0;test(safe_dose(-1,625,progress)&&highs==0&&!rng_running,"dose negative target");
 test(steps_calc_safe(10,0,0,1)==0,"zero cal steps");
 test(delay_us_calc_safe(100,0,0,1)==0,"zero cal delay");
 test(delay_us_calc_safe(0,0,5000,1)==0,"zero speed");
 test(steps_calc_safe(10,0,5000,1)==4800,"normal steps");
 test(delay_us_calc_safe(100,0,5000,1)==625,"normal delay");
 test(delay_us_calc_safe(3,0,5000,1)==0,"slow speed");
 test(steps_calc_safe(9999,0,1,1)==0,"long overflow");
 std::printf("HOST_CPP_CANDIDATE_OUTPUT_TESTS=%d PASS; AVR_ARDUINO_BUILD=NOT_RUN\n",passed);
}
