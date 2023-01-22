/* Functions to handle all input elements */

#include "mainSettings.h"
#include "Switch.h"
#include "Encoder.h"
// Activate general trace output

#ifdef TRACE_ON
#define TRACE_INPUT 
#define TRACE_INPUT_ENCODER
#endif

#define SHOW_KEYPRESS_ON_BUILTIN

/******** Encoder configuration  ********/

// CLOCK PIN must be handled by interrupt
#define ENCODER_CLOCK_PIN 2
#define ENCODER_DIRECTION_PIN 4

// Press function is managed as normal switch

/********* Switch configuration *********/
#define ENCODER_SWITCH_PIN 5
byte input_keyboard_pin[]={3,6,7};
#define KEYBOARD_BUTTON_COUNT 3

/* *********** General state variables of the input module ************ */
unsigned long input_last_event_time = 0;
bool input_ignore_until_release_flag=true;

/******** Encoder variables  ********/

Encoder myEncoder;

/****** switch constants and variables ******/

Switch input_encoderButton;
Switch input_keyboardButton[KEYBOARD_BUTTON_COUNT];

/* ***************************       S E T U P           ******************************
*/

void input_setup() {


  /* Initalize the encoder */
  pinMode(ENCODER_CLOCK_PIN,INPUT);
  pinMode(ENCODER_DIRECTION_PIN,INPUT);
  myEncoder.configureCloseSignal(LOW);  // We have a pullup circuit. So a falling flank initiates the turn
  myEncoder.configureRange(4, 83, 4, ENCODER_H_WRAP); 
  myEncoder.setValue(12);

  
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLOCK_PIN),encoder_clock_change_ISR,CHANGE);

  /* Initialize the switches */
  pinMode(ENCODER_SWITCH_PIN,INPUT_PULLUP);
  input_encoderButton.configureCloseSignal(LOW); // this circuit has a pullup logic
  for(byte s=0;s<KEYBOARD_BUTTON_COUNT;s++) {
    pinMode(input_keyboard_pin[s],INPUT_PULLUP);
    input_keyboardButton[s].configureCloseSignal(LOW); // pullup logic
  }

  /* Setup differenc debounce duration just for testing (can be removed) */
  int final_debounce_duration=0;
  final_debounce_duration=input_keyboardButton[0].configureDebounceWaittime(1000); 
  #ifdef TRACE_INPUT 
    Serial.print(F("TRACE_INPUT: final debounce durations 0="));
    Serial.print(final_debounce_duration);
  #endif
  final_debounce_duration=input_keyboardButton[1].configureDebounceWaittime(87);
  #ifdef TRACE_INPUT 
    Serial.print(F("  1="));
    Serial.print(final_debounce_duration);
  #endif
  final_debounce_duration=input_keyboardButton[2].configureDebounceWaittime(20);
  #ifdef TRACE_INPUT 
    Serial.print(F("  2="));
    Serial.println(final_debounce_duration);
  #endif

}

/* ********************************************************************************************************** */
/*               Interface functions                                                                          */

/* ******************* General Information functions******************** */

int input_getSecondsSinceLastEvent() {
  unsigned long timestamp_difference = (millis() - input_last_event_time) / 1000;
  if (timestamp_difference > 255) return 255;
#ifdef TRACE_INPUT_TIMING
  Serial.print(F("TRACE_INPUT_TIMING:input last interaction:"));
  Serial.println(timestamp_difference);
#endif
  return timestamp_difference;
}

/*!
  determines if input events and states can be used or should be ignored (e.g. waiting for releas of all buttons)
  @return true is
*/
bool input_is_valid() {
  return !input_ignore_until_release_flag;
}

/*!
  triggers the ignore mechanics. This has no effect on input scanning and tracking but will set input_is_valid to false
  until all relevant switches are back open again.
 @param activate boolean if ignore should be started  
*/
void input_ignore_until_release(bool activate) {
  input_ignore_until_release_flag=activate;
}

/* ******************* General scan function ********************
* call all scan functions and keeps track of last_event timestamp
*/

void input_scan_tick()
{
  if(myEncoder.processChange() || input_switch_scan()) input_last_event_time = millis(); // Reset the global age of interaction

  if(input_ignore_until_release_flag) { // check if any relevant switches ist pressed
    if(input_encoderButton.isClosed()) return;
    for(byte s=0;s<KEYBOARD_BUTTON_COUNT;s++) if(input_keyboardButton[s].isClosed()) return;
    
  }
  input_ignore_until_release_flag=false; // if not, we can reset the ignore flag

} // input_scan_tick




/* ************* switch state functions *************** */
/* This will be adapted to the usecase for better clarity in the functional code */

bool input_keyGotPressed(byte k) { return input_keyboardButton[k].gotClosed(); };
bool input_keyIsPressed(byte k) { return input_keyboardButton[k].isClosed(); };
uint16_t input_keyGetPressDuration(byte k) { return input_keyboardButton[k].getClosedDuration(); };
uint16_t input_keyGetReleaseDuration(byte k) { return input_keyboardButton[k].getOpenDuration(); };
bool input_keyGotReleased(byte k) { return input_keyboardButton[k].gotOpened(); };
bool input_keyIsReleased(byte k) { return input_keyboardButton[k].isOpen(); };

/* ***************** Encoder state functions ************************* */

bool input_encoder_gotPressed() {
  return input_encoderButton.gotClosed();
}

bool input_encoder_isPressed() {
  return input_encoderButton.isClosed();
}

bool input_encoder_gotReleased() {
  return input_encoderButton.gotOpened();
}

bool input_encoder_isReleased() {
  return input_encoderButton.isOpen();
}

bool input_encoder_hasPendingChange() {  
  return myEncoder.hasPendingChange();
}

int input_encoder_getValue() {
  return myEncoder.getValue();
}

int input_encoder_setValue(int newValue) {
  return myEncoder.setValue(newValue);
}

int input_encoder_enable() {
  myEncoder.enable();
}

int input_encoder_disable() {
  myEncoder.disable();
}

/* ***********  Encoder tracking function **************** */

void encoder_clock_change_ISR() {
  myEncoder.processSignal(digitalRead(ENCODER_CLOCK_PIN),digitalRead(ENCODER_DIRECTION_PIN));
}

/* *************** switch scan ****************** */

bool input_switch_scan() {
  bool has_change=false;
  #ifdef SHOW_KEYPRESS_ON_BUILTIN
  bool press_tracer=false;
  #endif

  has_change|=input_encoderButton.processSignal(digitalRead(ENCODER_SWITCH_PIN)); 
  #ifdef SHOW_KEYPRESS_ON_BUILTIN
    press_tracer|=input_encoderButton.isClosed();
  #endif

  for(byte s=0;s<KEYBOARD_BUTTON_COUNT;s++){
    has_change|=input_keyboardButton[s].processSignal(digitalRead(input_keyboard_pin[s])); 
    #ifdef SHOW_KEYPRESS_ON_BUILTIN
      press_tracer|=input_keyboardButton[s].isClosed();
    #endif
  } 
  #ifdef SHOW_KEYPRESS_ON_BUILTIN
    if(press_tracer) digitalWrite(LED_BUILTIN,HIGH);
    else digitalWrite(LED_BUILTIN,LOW);
  #endif
  return has_change;
}






