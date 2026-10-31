// PerPompa v1.05 P0-01: motor output host regression fixture.
// Original dose()/pump() BODY copied unchanged from firmware blob 0d965c08947ba0b3170d22fbff806fc2e6f1e83a.
// Host GPIO and delay mocks only. Not an AVR, driver, hardware, flow or stop-latency test.
// Run: g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-sanitize-recover=all p0-01-step-baseline.cpp -o p0-01-step-baseline && ./p0-01-step-baseline
#include <cassert>
#include <cstdio>
#define MOTOR_STEP_PIN 7
#define HIGH 1
#define LOW 0
int stepHigh=0, stepLow=0, delays=0, rngWrites=0;
bool rng_running=false;
void digitalWrite(int pin,int level){if(pin==MOTOR_STEP_PIN){if(level==HIGH)stepHigh++;else stepLow++;}}
void delayMicroseconds(unsigned int){delays++;}
void set_rng_running(bool running){if(rng_running==running)return;rng_running=running;rngWrites++;}

using boolean=bool;
boolean dose(long _steps, int _delay_us, long & inc) {
      if(inc < _steps){
        set_rng_running(true);
        digitalWrite(MOTOR_STEP_PIN,HIGH);
        delayMicroseconds(_delay_us);
        digitalWrite(MOTOR_STEP_PIN,LOW);
        delayMicroseconds(_delay_us);
        inc++;
        return false;
      } else {
        set_rng_running(false);
        inc=0;
        return true;
      }
}
void pump(int _delay_us) {
        set_rng_running(true);
        digitalWrite(MOTOR_STEP_PIN,HIGH);
        delayMicroseconds(_delay_us);
        digitalWrite(MOTOR_STEP_PIN,LOW);
        delayMicroseconds(_delay_us);
}
void reset(){stepHigh=stepLow=delays=rngWrites=0;rng_running=false;}
int main(){
 reset(); long progress=0;
 assert(dose(2,625,progress)==false && stepHigh==1 && stepLow==1 && progress==1 && rng_running);
 assert(dose(2,625,progress)==false && stepHigh==2 && stepLow==2 && progress==2 && rng_running);
 assert(dose(2,625,progress)==true && stepHigh==2 && stepLow==2 && progress==0 && !rng_running);
 puts("HOST_CPP_DOSE_POSITIVE=PASS");
 reset(); pump(625);assert(stepHigh==1 && stepLow==1 && rng_running);puts("HOST_CPP_PUMP_POSITIVE=PASS");
 reset(); pump(0);assert(stepHigh==1 && stepLow==1 && rng_running);puts("HOST_CPP_INVALID_ZERO_PUMP_PULSES=REPRODUCED");
 reset();progress=0;assert(!dose(1,0,progress));assert(stepHigh==1 && stepLow==1 && rng_running);puts("HOST_CPP_INVALID_ZERO_DOSE_PULSES=REPRODUCED");
 reset();progress=0;assert(dose(0,0,progress) && stepHigh==0 && !rng_running);puts("HOST_CPP_ZERO_VOLUME_NO_PULSES=PASS");
}
