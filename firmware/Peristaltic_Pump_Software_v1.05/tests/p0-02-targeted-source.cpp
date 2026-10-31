// P0-02 source-excerpt host harness: pump(), dose(), handle_remote_input()
// from accepted PerPompa v1.05. Does NOT measure physical AVR latency.
#include <cstdio>
#include <cstdlib>
#include <cstdint>
using boolean=bool;
#define HIGH 1
#define LOW 0
#define MOTOR_STEP_PIN 7
#define REMOTE_PIN 5
#define REMOTE_HOLD 0
#define REMOTE_PULSE 1
#define REMOTE_DEBOUNCE_MS 30
int pins[32]{}; long stepCounter=0; unsigned long mockMs=0,mockUs=0;
long delay_us=16000;bool usb_start=false, rng_running=false, in_menu=false,in_action=false, hwUiActive=false,remote_started=false,remote_armed=true;
bool remote_last_read=HIGH,remote_stable_state=HIGH;unsigned long remote_last_change_ms=0;
int menu_number_1=0;int startEvents=0,stopEvents=0,highEvents=0;bool allow=true;
struct Menu{int value=0;} menu[12];
int digitalRead(int pin){return pins[pin];}
void digitalWrite(int pin,int value){pins[pin]=value;if(pin==MOTOR_STEP_PIN&&value==HIGH)++highEvents;}
unsigned long millis(){return mockMs;}
void delayMicroseconds(unsigned int us){mockUs+=us;mockMs=mockUs/1000;}
void set_rng_running(bool val){rng_running=val;}
bool hwMotorAllowed(){return allow;}
void hwForceIdle(){digitalWrite(MOTOR_STEP_PIN,LOW);set_rng_running(false);usb_start=false;}
void exit_action_menu(){in_action=false;hwForceIdle();}
void remote_start(){in_action=true;remote_started=true;++startEvents;}
void remote_stop(){in_action=false;remote_started=false;hwForceIdle();++stopEvents;}

// Exact accepted pump() body; source checked by GitHub targeted gate.
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
// Exact accepted dose() body.
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
// Exact accepted remote debounce body.
void handle_remote_input(){
  bool reading = digitalRead(REMOTE_PIN);
  if (reading != remote_last_read){
    remote_last_change_ms = millis();
    remote_last_read = reading;
  }
  if (reading == remote_stable_state || millis() - remote_last_change_ms < REMOTE_DEBOUNCE_MS) return;

  remote_stable_state = reading;
  // If the contact was LOW at boot, a debounced HIGH must be observed first.
  if (!remote_armed){
    if (reading == HIGH) remote_armed = true;
    return;
  }
  if (menu[10].value == REMOTE_HOLD){
    if (reading == LOW){
      remote_start();
    } else if (remote_started){
      remote_stop();
    }
  } else if (reading == LOW){
    // In Pulse mode each new contact closure toggles the Start action.
    if (in_action && menu_number_1 == 0){
      remote_stop();
    } else {
      remote_start();
    }
  }
}
void check(bool ok,const char *s){if(!ok){std::fprintf(stderr,"P002_HOST_FAIL: %s\n",s);std::exit(1);}}
int main(){
  int n=0; pins[REMOTE_PIN]=HIGH;menu[10].value=REMOTE_HOLD;
  pump(4);check(mockUs==8&&highEvents==1,"pump min pulses 8us delay calls");++n;
  mockUs=mockMs=0;highEvents=0;pump(16000);
  check(mockUs==32000&&highEvents==1,"pump 32ms blocked delay at max accepted speed");++n;
  mockUs=mockMs=0;highEvents=0;long counter=0;check(!dose(1,16000,counter),"dose single accepted step");++n;
  check(mockUs==32000&&counter==1&&highEvents==1,"dose also waits 32ms/step");++n;
  mockUs=mockMs=0;highEvents=0;allow=false;counter=0;
  check(dose(4,16000,counter)&&highEvents==0&&!rng_running,"denied dose no pulse");++n;
  allow=true;in_action=true;remote_started=true;remote_stable_state=LOW;remote_last_read=LOW;
  pins[REMOTE_PIN]=HIGH;mockMs=0;handle_remote_input();
  check(stopEvents==0&&in_action,"remote release not processed before debounce");++n;
  mockMs=29;handle_remote_input();check(stopEvents==0,"remote still waiting at 29ms");++n;
  mockMs=30;handle_remote_input();check(stopEvents==1&&!in_action&&!rng_running,"remote stop at 30ms stable");++n;
  std::printf("P002_TARGETED_SOURCE_EXCERPT_CPP=%d/%d PASS; PULSE_WAIT_MAX=32000us; REMOTE_DEBOUNCE=30ms; REAL_AVR_TIMING=NOT_RUN; RELEASE_BLOCKED\n",n,n);
}
