// include the library code:
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
#include <ClickEncoder.h> //https://github.com/0xPIT/encoder/blob/master/README.md -> Encoder processing (timer based)
#include <TimerOne.h> //required for ClickEncoder.h 
#include <EEPROM.h> //write and read EEPROM (to save and load settings)
#include <stdio.h>
#include <string.h>
#include "hw-setup-runtime.h"
#include "p0-02-usb-parser.h"
#include "p0-04-settings-store.h"
#include "p004-migration-guard-candidate.h"
bool saveSettingsAtomic();


//LCD -----------------------------------------------------------------------------------
#define LCD_COLUMNS 16
#define LCD_ROWS 2
// I2C LCD: SDA=A4 / SCL=A5; hd44780_I2Cexp auto-discovers PCF8574 backpack.
hd44780_I2Cexp lcd;
bool lcd_init_ok = false;

//ENCODER --------------------------------------------------------------------------------
#define ENCODER_PIN_BUTTON 2
#define ENCODER_PIN_A 3
#define ENCODER_PIN_B 4
#define ENCODER_STEPS_PER_NOTCH 4
#define ENCODER_MIN_STEP_MS 5
#define ENCODER_REVERSE_GUARD_MS 25
#define ENCODER_BUTTON_GUARD_MS 80
ClickEncoder *encoder;
int8_t encoder_step = 0;
int8_t encoder_last_direction = 0;
unsigned long encoder_last_step_ms = 0;
unsigned long encoder_button_low_ms = 0;
unsigned long encoder_button_event_ms = 0;
bool encoder_button_pending = false;
bool encoder_hold_handled = false;

//REMOTE START/STOP -----------------------------------------------------------------------
#define REMOTE_PIN 5
#define REMOTE_HOLD 0
#define REMOTE_PULSE 1
#define REMOTE_DEBOUNCE_MS 30
bool remote_last_read = HIGH;
bool remote_stable_state = HIGH;
bool remote_started = false;
bool remote_armed = false; // Require stable released input before first remote start.
unsigned long remote_last_change_ms = 0;

//RNG RUNNING STATUS ----------------------------------------------------------------------
// A0 is a digital-capable pin on the Pro Mini. Use a relay driver/module, not a bare coil.
// This signal reports commanded motor stepping, not measured rotation or liquid flow.
// Swap ACTIVE and IDLE levels below if the relay module is active LOW.
#define RNG_PIN A0
#define RNG_ACTIVE_LEVEL HIGH
#define RNG_IDLE_LEVEL LOW
bool rng_running = false;

//STEP MOTOR -----------------------------------------------------------------------------
#define MOTOR_STEP_PIN 7
#define MOTOR_DIR_PIN 6
#define STEP_MODE 4 // (1: Full Step, 2: Half Step, 4: Quarter Step, ...)
#define STEPS_PER_FULL_ROT 200 // @ full steps
long delay_us;
long steps;
long step_counter = 0;

//CALIBRATION -----------------------------------------------------------------------------
#define CALIBR_ROTATIONS 30
#define CALIBR_DURATION 30 // seconds
#define CALIBR_DECIMALS 3
const int CALIBR_DECIMAL_CORR = pow(10,CALIBR_DECIMALS);

//SERIAL COMMUNICATION ---------------------------------------------------------------------
#define BAUD 9600
String inputString = "";         // a String to hold incoming data
boolean stringComplete = false;  // whether the string is complete
long vol_uL=0;
long rate_uL_min =0;
int cal=0;
boolean usb_start=0;
perpompa_usb::Parser usbParser;
bool usbDropUntilEmpty=false;
char inChar;


//STATE ------------------------------------------------------------------------------------
boolean in_menu=0;
volatile boolean in_action=0;
boolean menu_entered=0;
boolean menu_left=0;

//GENERAL -----------------------------------------------------------------------------------
#define MICROSEC_PER_SEC 1000000

const unsigned int CALIBR_STEPS = CALIBR_ROTATIONS * STEPS_PER_FULL_ROT * STEP_MODE;
const unsigned int CALIBR_DELAY_US = (CALIBR_DURATION * MICROSEC_PER_SEC)/(CALIBR_STEPS*2);


//MENU ---------------------------------------------------------------------------------------
#define MAX_NUM_OF_OPTIONS 4
#define NUM_OF_MENU_ITEMS 12
#define VALUE_MAX_DIGITS 4
int menu_number_1=0;
int menu_number_2=1;
boolean val_change =0;
bool hwUiActive=false;
perpompa_hw::SafeSetup<EEPROMClass> hwSetup(EEPROM,F_CPU);
perpompa_p004::Store<EEPROMClass> settingsStore(EEPROM);
p004_migration_candidate::Gate<EEPROMClass> migrationGate(EEPROM);
p004_migration_candidate::BootKind migrationBoot=p004_migration_candidate::kLocked;
bool migrationReviewAccepted=false;
double value_dbl;
char value_str[16]; // dtostrf width is a minimum; displayed values can exceed four characters.
char lcd_shown[2][LCD_COLUMNS+1];
bool lcd_cache_valid = false;

enum menu_type {
  VALUE,
  OPTION,
  ACTION
};

typedef struct 
{
  const char* name_;
  menu_type type; //0: value type, 1:option type, 2:action type
  int value;
  int decimals;
  int lim;
  const char* options[4];
  const char* suffix;
}menu_item;
int menu_items_limit = NUM_OF_MENU_ITEMS-1;
menu_item menu[NUM_OF_MENU_ITEMS];


//███ SETUP ████████████████████████████████████████████████████████████████████████████████████████████████████
void setup(){
  
 pinMode(MOTOR_STEP_PIN,OUTPUT);
 hwSetup.boot();
 pinMode(MOTOR_DIR_PIN,OUTPUT);
 digitalWrite(RNG_PIN, RNG_IDLE_LEVEL);
 pinMode(RNG_PIN, OUTPUT);
 pinMode(REMOTE_PIN, INPUT_PULLUP);
 remote_last_read = digitalRead(REMOTE_PIN);
 remote_stable_state = remote_last_read;
 remote_armed = (remote_stable_state == HIGH);
 digitalWrite(MOTOR_DIR_PIN,LOW);
 digitalWrite(MOTOR_STEP_PIN,LOW);

 menu[0].name_ = "Start";
 menu[0].type = ACTION;
 menu[0].value = 0;
 menu[0].lim = 0;
 menu[0].suffix = "RUNNING!";

 menu[1].name_ = "Volume";
 menu[1].type = VALUE;
 menu[1].value = 0;
 menu[1].decimals = 1;
 menu[1].lim = 9999;
 menu[1].suffix="mL";

 menu[2].name_ = "V.Unit:";
 menu[2].type = OPTION;
 menu[2].value = 0;
 menu[2].lim = 3-1;
 menu[2].options[0] = "mL";
 menu[2].options[1] = "uL";
 menu[2].options[2] = "rot";

 menu[3].name_ = "Speed";
 menu[3].type = VALUE;
 menu[3].value = 0;
 menu[3].decimals = 1;
 menu[3].lim = 999;
 menu[3].suffix="mL/min";

 menu[4].name_ = "S.Unit:";
 menu[4].type = OPTION;
 menu[4].value = 0;
 menu[4].lim = 3-1;
 menu[4].options[0] = "mL/min";
 menu[4].options[1] = "uL/min";
 menu[4].options[2] = "rpm";

 menu[5].name_ = "Direction:";
 menu[5].type = OPTION;
 menu[5].value = 0;
 menu[5].lim = 2-1;
 menu[5].options[0] = "CW";
 menu[5].options[1] = "CCW";

 menu[6].name_ = "Mode:";
 menu[6].type = OPTION;
 menu[6].value = 0;
 menu[6].lim = 3-1;
 menu[6].options[0] = "Dose";
 menu[6].options[1] = "Pump";
 menu[6].options[2] = "Cal.";

 menu[7].name_ = "Cal.";
 menu[7].type = VALUE;
 menu[7].value = 0;
 menu[7].decimals = CALIBR_DECIMALS;
 menu[7].lim = 20000;
 menu[7].suffix="mL";

 menu[8].name_ = "Save Sett.";
 menu[8].type = ACTION;
 menu[8].value = 0;
 menu[8].lim = 0;
 menu[8].suffix = "OK!";

 menu[9].name_ = "USB Ctrl";
 menu[9].type = ACTION;
 menu[9].value = 0;
 menu[9].lim = 0;
 menu[9].suffix = "ON!";

 menu[10].name_ = "Remote:";
 menu[10].type = OPTION;
 menu[10].value = REMOTE_PULSE;
 menu[10].lim = REMOTE_PULSE;
 menu[10].options[REMOTE_HOLD] = "Hold";
 menu[10].options[REMOTE_PULSE] = "Pulse";

 menu[11].name_ = "HW Setup";
 menu[11].type = ACTION;
 menu[11].value = 0;
 menu[11].lim = 0;
 menu[11].suffix = "";

  int storedValues[perpompa_p004::kItems]={};
  migrationBoot=migrationGate.boot(storedValues);
  for (int i=0; i < 11; i++){
       int saved_value = migrationBoot == p004_migration_candidate::kNewReady ? storedValues[i] :
          (migrationBoot == p004_migration_candidate::kLegacyNeedsReview ? eepromReadInt(i*2) : -1);
      if (menu[i].type == ACTION){
        menu[i].value = 0;
      } else if (saved_value >= 0 && saved_value <= menu[i].lim){
        menu[i].value = saved_value;
      } else {
        // EEPROM Remote slot: fail to Pulse, not Hold on invalid/erased data.
        menu[i].value = (i == 10) ? REMOTE_PULSE : 0;
      }
  }
  // The new EEPROM slot may contain an uninitialized value on existing pumps.
  // Pulse requires a new closure, so it avoids an immediate start on first use.
  if (menu[10].value < REMOTE_HOLD || menu[10].value > REMOTE_PULSE){
    menu[10].value = REMOTE_PULSE;
  }
  
  encoder = new ClickEncoder(ENCODER_PIN_B, ENCODER_PIN_A, ENCODER_PIN_BUTTON, ENCODER_STEPS_PER_NOTCH); //(Encoder A, Encoder B, PushButton)
  encoder->setAccelerationEnabled(false);
  Timer1.initialize(1000);
  Timer1.attachInterrupt(timerIsr);
  encoder_button_event_ms = millis();
  
  Serial.begin(BAUD);
  inputString.reserve(200);
  // set up the LCD's number of columns and rows:
  lcd_init_ok = (lcd.begin(LCD_COLUMNS, LCD_ROWS) == 0);
  if (!lcd_init_ok) { hwForceIdle(); return; }
  // Print a message to the LCD.
  menu[1].suffix = menu[2].options[menu[2].value];
  if (strcmp(menu[1].suffix,"uL")==0){
    menu[1].decimals = 0;
  } else {
    menu[1].decimals = 1;
  }
  menu[3].suffix = menu[4].options[menu[4].value];
  if (menu[4].value == 1){
    menu[3].decimals = 0;
  } else {
    menu[3].decimals = 1;
  }
  update_lcd();
  steps = steps_calc(menu[1].value, menu[2].value, menu[7].value, menu[1].decimals);
  delay_us = delay_us_calc(menu[3].value, menu[4].value, menu[7].value, menu[3].decimals);
  // Boot is fail-closed: even a saved Hold mode requires release and fresh closure.
  
}



//███ LOOP ████████████████████████████████████████████████████████████████████████████████████████████████████

void loop() {
  if (!lcd_init_ok) { hwForceIdle(); return; }
  
// BUTTON HANDLING ////////////////////////////////////////////////////////////////////////////////

bool encoder_button_down = (digitalRead(ENCODER_PIN_BUTTON) == LOW);
if (encoder_button_down){
  encoder_button_pending = true;
  encoder_button_low_ms = millis();
}
ClickEncoder::Button b = encoder->getButton();
  if (menu_number_1 == 11 && b != ClickEncoder::Open) {
    if (b == ClickEncoder::Clicked) {
      if (!hwUiActive) {
        hwUiActive=hwSetup.enter(!in_action && !rng_running,
          !remote_started && remote_stable_state==HIGH, !usb_start);
      } else {
        hwSetup.nextPage();
      }
      val_change=true;
    } else if (b == ClickEncoder::Held && hwUiActive && !encoder_hold_handled) {
      if (hwSetup.page()==perpompa_hw::kConfirmPage) {
        if (hwSetup.confirmAndSave(true, !in_action && !rng_running,
          !remote_started && remote_stable_state==HIGH, !usb_start)) hwUiActive=false;
      } else { hwSetup.cancel(); hwUiActive=false; }
      encoder_hold_handled=true; val_change=true;
    } else if (b == ClickEncoder::DoubleClicked && hwUiActive) {
      hwSetup.cancel(); hwUiActive=false; val_change=true;
    } else if (b == ClickEncoder::Released) {
      encoder_hold_handled=false;
    }
  } else if (b != ClickEncoder::Open) {
    if (b == ClickEncoder::Clicked || b == ClickEncoder::DoubleClicked || b == ClickEncoder::Released){
      encoder_button_pending = false;
      encoder_button_event_ms = millis();
      encoder_last_direction = 0;
    }
    switch (b) {
      case ClickEncoder::Open:
      case ClickEncoder::Pressed:
        break;
      case ClickEncoder::Clicked:
        if(menu[menu_number_1].type == VALUE ||menu[menu_number_1].type == OPTION){ // if value or option type
          in_menu =!in_menu;
        }
        if(menu[menu_number_1].type == ACTION){ // if action type
          in_action=!in_action;
          step_counter= 0;
        }
        
        if (in_action == true ||in_menu ==true){ //menu entered
          menu_entered = true;
        }
        
        if (in_action == false && in_menu ==false){ //menu left
          menu_left = true;
        }
        break;
        
      case ClickEncoder::DoubleClicked:
          if (in_menu && menu[menu_number_1].type == VALUE){
          int increment = menu[menu_number_1].lim/10;
          if (menu[menu_number_1].value > menu[menu_number_1].lim - increment){
            menu[menu_number_1].value = menu[menu_number_1].lim;
          } else {
            menu[menu_number_1].value += increment;
          }
          val_change=true;
          }
        break;
      case ClickEncoder::Held:
          // Explicit local HOLD after the legacy review prompt; USB w and remote never acknowledge migration.
          if(!encoder_hold_handled && menu_number_1==8 && in_action &&
             migrationBoot==p004_migration_candidate::kLegacyNeedsReview &&
             !hwUiActive && !rng_running && !usb_start && !remote_started &&
             remote_stable_state==HIGH && menu[7].value>0) migrationReviewAccepted=true;
          if (!encoder_hold_handled && in_menu && menu[menu_number_1].type == VALUE){
          menu[menu_number_1].value = 0;
          val_change=true;
          }
          encoder_hold_handled = true;
        break;
      case ClickEncoder::Released:
        encoder_hold_handled = false;
        break;
        }
  }
encoder_step = read_encoder_step(); // always drains movement, including while the pump is running
if(hwUiActive && encoder_step != 0){
  if(hwSetup.page() == perpompa_hw::kBoardPage){
    hwSetup.chooseBoard(encoder_step > 0 ? perpompa_hw::kProMini5V16MHz : perpompa_hw::kProMini3V3_8MHz);
  } else if(hwSetup.page() == perpompa_hw::kDriverPage){
    hwSetup.chooseDriver(encoder_step > 0 ? perpompa_hw::kDRV8825 : perpompa_hw::kA4988);
  }
  encoder_step=0; val_change=true;
}


/// SETUP ////////////////////////////////////////////////////////////////////////////////

if (menu_entered){
  lcd.blink();
  if (menu[menu_number_1].type == ACTION){
    lcd.setCursor((LCD_COLUMNS - strlen(menu[menu_number_1].suffix)), 0);
    lcd.print(menu[menu_number_1].suffix);
    lcd.setCursor(15, 0);
    lcd_cache_valid = false;
  }
  // Acceleration stays disabled: each accepted detent changes a value by one step.
  menu_entered = false;
}



/// ACTIONS ////////////////////////////////////////////////////////////////////////////////

if (in_action){
  switch (menu_number_1){
  case 0: //Start
  if (menu[6].value < 0 || menu[6].value > 2) { hwForceIdle(); exit_action_menu(); break; }
  if (menu[6].value == 0){ //Dose
    if (steps <= 0 || delay_us < 4 || delay_us > 16000 || dose(steps, delay_us, step_counter)){
      exit_action_menu();
    }
  } else if (menu[6].value == 1){ //Pump
    if (delay_us < 4 || delay_us > 16000) exit_action_menu();
    else pump(delay_us);
  } else if (menu[6].value == 2){ //Cal.
    if (dose(CALIBR_STEPS, CALIBR_DELAY_US, step_counter)){
      exit_action_menu();
    }
  }
  break;

  case 8:
   if(migrationBoot==p004_migration_candidate::kLegacyNeedsReview && !migrationReviewAccepted){
     hwForceIdle();lcd.setCursor(0,0);lcd.print("VERIFY CAL SETS ");
     lcd.setCursor(0,1);lcd.print("HOLD TO CONFIRM ");break;
   }
   if (!saveSettingsAtomic()){hwForceIdle();exit_action_menu();break;}
   delay(700);
   menu_left = true;
  break;
  
  case 9: {
    // Bounded, nonblocking input. STOP events are processed before any STEP.
    uint8_t budget=32;
    while (budget && Serial.available()) {
      --budget;
      if (usbDropUntilEmpty) { (void)Serial.read(); continue; }
      perpompa_usb::Event e=usbParser.feed((uint8_t)Serial.read(),millis());
      if (e.kind!=perpompa_usb::kNone) usbApplyEvent(e);
    }
    if (usbDropUntilEmpty && !Serial.available()) usbDropUntilEmpty=false;
    if (Serial.available()) {
      // Fail closed on excess queued data; discard across future loops.
      usbParser.reset(); usbDropUntilEmpty=true; hwForceIdle(); inChar=0;
    }
    // Process UART input before quiet completion. An already-queued STOP or
    // late final digit must never be overtaken by a stale timeout event.
    if (!usbDropUntilEmpty && !Serial.available()) {
      perpompa_usb::Event idleEvent=usbParser.tick(millis());
      if (idleEvent.kind!=perpompa_usb::kNone) usbApplyEvent(idleEvent);
    }


  if (usb_start) {
    if ((inChar == 'p' && (delay_us < 4 || delay_us > 16000)) ||
        (inChar == 'd' && (steps <= 0 || delay_us < 4 || delay_us > 16000))) {
      usb_start = false;
      set_rng_running(false);
    } else if(inChar == 'p'){
      pump(delay_us);
    } else if (inChar == 'd') {
      if (dose(steps, delay_us, step_counter)){
        usb_start = false;
      }
    } else if (inChar == 'c'){
      if (dose(CALIBR_STEPS, CALIBR_DELAY_US, step_counter)){
        usb_start = false;
      }
    }
  }
  break;
  }
}

/// MENU (no action) ////////////////////////////////////////////////////////////////////////////////

} else if (!in_action){
if (!in_menu){ // no menu selected
  if (encoder_selection(menu_number_1, menu_number_2, menu_items_limit)) val_change = true;

}else if(in_menu){ // menu selected
  if(menu[menu_number_1].type == 0){
    if (encoder_value_selection(menu[menu_number_1].value, menu[menu_number_1].lim)) val_change = true;
  } else {
    if (encoder_selection(menu[menu_number_1].value, menu[menu_number_1].lim)) val_change = true;
  }
}

}

/// CLOSE ////////////////////////////////////////////////////////////////////////////////

if (menu_left){
  lcd.noBlink();
  if (menu[menu_number_1].type == ACTION){
    exit_action_menu();
  }
  
  if (menu_number_1 == 2){
    menu[menu_number_1-1].suffix = menu[menu_number_1].options[menu[menu_number_1].value];
      if (strcmp(menu[1].suffix,"uL")==0){
        menu[1].decimals = 0;
      } else {
        menu[1].decimals = 1;
       }
  }

  if (menu_number_1 == 4){
    menu[menu_number_1-1].suffix = menu[menu_number_1].options[menu[menu_number_1].value];
    if (strcmp(menu[3].suffix,"uL/min")==0){
      menu[3].decimals = 0;
    } else {
      menu[3].decimals = 1;
    }
  }
  if (menu_number_1 == 5){ //Change Direction
    if (menu[5].value == 0){
      digitalWrite(MOTOR_DIR_PIN,LOW); 
    }else{
      digitalWrite(MOTOR_DIR_PIN,HIGH);
    }
  }
  steps = steps_calc(menu[1].value, menu[2].value, menu[7].value, menu[1].decimals);
  delay_us = delay_us_calc(menu[3].value, menu[4].value, menu[7].value, menu[3].decimals);
  
  menu_left = false;
  val_change = true;
}

if (val_change){
  update_lcd();
  val_change = false;
}

handle_remote_input();

} 

//███ FUNCTION DECLARATION █████████████████████████████████████████████████████████████████████████████████████████████████

//_____________________________________________________________________________________________

void timerIsr() {
  encoder->service();
}

//_____________________________________________________________________________________________

int8_t read_encoder_step(){
  int16_t raw = encoder->getValue();
  if (raw == 0) return 0;

  unsigned long now = millis();
  if (encoder_button_pending && digitalRead(ENCODER_PIN_BUTTON) == HIGH &&
      now - encoder_button_low_ms > 700){
    encoder_button_pending = false;
  }
  if (in_action || digitalRead(ENCODER_PIN_BUTTON) == LOW || encoder_button_pending ||
      now - encoder_button_event_ms < ENCODER_BUTTON_GUARD_MS){
    return 0;
  }

  int8_t direction = (raw > 0) ? 1 : -1;
  unsigned long elapsed = now - encoder_last_step_ms;
  if (encoder_last_direction != 0){
    if (elapsed < ENCODER_MIN_STEP_MS) return 0;
    if (direction != encoder_last_direction && elapsed < ENCODER_REVERSE_GUARD_MS) return 0;
  }
  encoder_last_direction = direction;
  encoder_last_step_ms = now;
  return direction;
}

//_____________________________________________________________________________________________

void set_rng_running(bool running){
  if (rng_running == running) return;
  rng_running = running;
  digitalWrite(RNG_PIN, running ? RNG_ACTIVE_LEVEL : RNG_IDLE_LEVEL);
}

//_____________________________________________________________________________________________

bool saveSettingsAtomic(){
  if(migrationBoot==p004_migration_candidate::kLocked ||
     (migrationBoot==p004_migration_candidate::kLegacyNeedsReview && !migrationReviewAccepted))return false;
  int values[perpompa_p004::kItems];
  for(uint8_t i=0;i<perpompa_p004::kItems;i++){
    if(menu[i].type!=ACTION && (menu[i].value<0 || menu[i].value>menu[i].lim))return false;
    values[i]=menu[i].value;
  }
  if(values[7]<=0)return false;
  const bool review=migrationBoot==p004_migration_candidate::kLegacyNeedsReview && migrationReviewAccepted;
  if(!migrationGate.save(values,review))return false;
  migrationBoot=migrationGate.boot(nullptr);
  migrationReviewAccepted=false;
  return migrationBoot==p004_migration_candidate::kNewReady;
}

void usbApplyEvent(const perpompa_usb::Event &e){
  if(migrationBoot!=p004_migration_candidate::kNewReady &&
     (e.kind==perpompa_usb::kPump || e.kind==perpompa_usb::kDose || e.kind==perpompa_usb::kCalibrate)){
    hwForceIdle();inChar=0;return;
  }
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
      {
        const int oldMenuCal=menu[7].value,oldCal=cal;
        cal=(int)e.value[0];menu[7].value=cal;
        if(!saveSettingsAtomic()){
          menu[7].value=oldMenuCal;cal=oldCal;
          hwForceIdle();inChar=0;return;
        }
      }
      return;
  }
  hwForceIdle(); inChar=0;
}

void hwForceIdle(){digitalWrite(MOTOR_STEP_PIN,LOW);set_rng_running(false);usb_start=false;}

bool hwMotorAllowed(){
  perpompa_hw::StartPath path=perpompa_hw::kStartPathCount;
  if(menu_number_1==9){
    path=inChar=='d' ? perpompa_hw::kUsbDose : inChar=='p' ? perpompa_hw::kUsbPump :
      inChar=='c' ? perpompa_hw::kUsbCal : perpompa_hw::kStartPathCount;
  } else if(menu_number_1==0){
    if(menu[6].value<0 || menu[6].value>2) return false;
    path=remote_started ? static_cast<perpompa_hw::StartPath>(perpompa_hw::kRemoteDose+menu[6].value) :
      static_cast<perpompa_hw::StartPath>(perpompa_hw::kLocalDose+menu[6].value);
  }
  return migrationBoot==p004_migration_candidate::kNewReady && hwSetup.permit(path,false);
}

void remote_start(){
  // Keep menu edits and the Save/USB actions under local control.
  if (in_menu || in_action || hwUiActive) return;
  if (migrationBoot!=p004_migration_candidate::kNewReady ||
       menu[6].value < 0 || menu[6].value > 2 ||
      !hwSetup.permit(static_cast<perpompa_hw::StartPath>(perpompa_hw::kRemoteDose+menu[6].value),false))
      { hwForceIdle(); return; }
  menu_number_1 = 0;
  menu_number_2 = 1;
  update_lcd();
  in_action = true;
  step_counter = 0;
  remote_started = true;
  menu_entered = true;
}

void remote_stop(){
  if (in_action && menu_number_1 == 0){
    exit_action_menu();
  }
  remote_started = false;
}

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

//_____________________________________________________________________________________________

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
//_____________________________________________________________________________________________
/*
void dose_slow(long _steps, int _delay_us, long & inc) {
      if(inc < _steps){
        digitalWrite(MOTOR_STEP_PIN,HIGH); 
        delay(_delay_us/1000); 
        digitalWrite(MOTOR_STEP_PIN,LOW); 
        delay(_delay_us/1000);
        inc++;
      } else {
        inc=0;
        exit_action_menu();
      }
}*/
//_____________________________________________________________________________________________

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
//_____________________________________________________________________________________________

void exit_action_menu(){
   migrationReviewAccepted=false;
   usbParser.reset();
   usbDropUntilEmpty=false;
   set_rng_running(false);
   // Leaving USB/Start actions must discard stale run state.
   usb_start = false;
   digitalWrite(MOTOR_STEP_PIN, LOW);
   in_action = false;
   lcd.setCursor((LCD_COLUMNS - strlen(menu[menu_number_1].suffix)), 0);
   lcd.print("          ");
   lcd.noBlink();
   lcd_cache_valid = false;
}
//_____________________________________________________________________________________________

long steps_calc(long volume, int unit_mode, int calibr, int decimals){
  if (volume <= 0 || unit_mode < 0 || unit_mode > 2 ||
      decimals < 0 || decimals > 3 || (unit_mode != 2 && calibr <= 0)) return 0;
  int decimal_corr = pow(10, decimals);
  double cal = ((double)calibr / CALIBR_ROTATIONS) / CALIBR_DECIMAL_CORR;
  double conv = unit_mode == 2 ? 1.0 : (unit_mode == 1 ? 1.0 / cal / 1000 : 1.0 / cal);
  double candidate = (double)STEPS_PER_FULL_ROT * STEP_MODE * conv * volume / decimal_corr;
  // On AVR double is 32-bit float. Keep a guard margin below signed-long overflow.
  if (!(candidate >= 1.0 && candidate <= 2147483000.0)) return 0;
  return (long)candidate;
}
//_____________________________________________________________________________________________

long delay_us_calc(long vol_per_min, int unit_mode, int calibr, int decimals){
  if (vol_per_min <= 0 || unit_mode < 0 || unit_mode > 2 ||
      decimals < 0 || decimals > 3 || (unit_mode != 2 && calibr <= 0)) return 0;
  int decimal_corr = pow(10, decimals);
  double cal = ((double)calibr / CALIBR_ROTATIONS) / CALIBR_DECIMAL_CORR;
  double conv = unit_mode == 2 ? 1.0 : (unit_mode == 1 ? 1.0 / cal / 1000 : 1.0 / cal);
  double candidate = (1.0 / (STEPS_PER_FULL_ROT * STEP_MODE * conv *
                     vol_per_min / decimal_corr)) * 60 * MICROSEC_PER_SEC / 2;
  // Fail closed: AVR int is 16-bit and overly long delayMicroseconds is not supported.
  if (!(candidate >= 4.0 && candidate <= 16000.0)) return 0;
  return (long)candidate;
}

//_____________________________________________________________________________________________

void format_menu_line(int index, char *line){
  char full[48];
  snprintf(full, sizeof(full), "%d|%s", index, menu[index].name_);
  if (menu[index].type == VALUE){
    value_dbl = menu[index].value / pow(10, menu[index].decimals);
    dtostrf(value_dbl, VALUE_MAX_DIGITS, menu[index].decimals, value_str);
    strncat(full, " ", sizeof(full) - strlen(full) - 1);
    strncat(full, value_str, sizeof(full) - strlen(full) - 1);
    strncat(full, menu[index].suffix, sizeof(full) - strlen(full) - 1);
  } else if (menu[index].type == OPTION){
    strncat(full, " ", sizeof(full) - strlen(full) - 1);
    strncat(full, menu[index].options[menu[index].value], sizeof(full) - strlen(full) - 1);
  }
  memset(line, ' ', LCD_COLUMNS);
  line[LCD_COLUMNS] = '\0';
  size_t count = strlen(full);
  if (count > LCD_COLUMNS) count = LCD_COLUMNS;
  memcpy(line, full, count);
}

void update_lcd(){
  if (menu_number_1 == 11){
    char top[17],bottom[17]; hwSetup.lcdRows(top,bottom);
    lcd.setCursor(0,0);lcd.print(top);lcd.setCursor(0,1);lcd.print(bottom);
    lcd_cache_valid=false; return;
  }
  char desired[2][LCD_COLUMNS+1];
  format_menu_line(menu_number_1, desired[0]);
  format_menu_line(menu_number_2, desired[1]);
  for (int row=0; row<LCD_ROWS; row++){
    for (int col=0; col<LCD_COLUMNS; col++){
      if (!lcd_cache_valid || desired[row][col] != lcd_shown[row][col]){
        lcd.setCursor(col, row);
        lcd.write(desired[row][col]);
        lcd_shown[row][col] = desired[row][col];
      }
    }
  }
  lcd_cache_valid = true;
  lcd.setCursor(1, 0);
}
//_____________________________________________________________________________________________

boolean encoder_selection(int & x, int lim){ //sub menu
  if (encoder_step == 0) return false;
  if (encoder_step > 0){
    x = (x >= lim) ? 0 : x + 1;
  } else {
    x = (x <= 0) ? lim : x - 1;
  }
  return true;
}
//_____________________________________________________________________________________________

boolean encoder_value_selection(int & x, int lim){ //sub menu
  return encoder_selection(x, lim);
}
//_____________________________________________________________________________________________

boolean encoder_selection(int & x, int & y, int lim){ //main menu
  if (!encoder_selection(x, lim)) return false;
  y = (x >= lim) ? 0 : x + 1;
  return true;
}
//_____________________________________________________________________________________________

int eepromReadInt(int adr) { 
//http://shelvin.de/eine-integer-zahl-in-das-arduiono-eeprom-schreiben/
byte low, high;

  low=EEPROM.read(adr);
  high=EEPROM.read(adr+1);
  return low + ((high << 8)&0xFF00);
} //eepromReadInt


