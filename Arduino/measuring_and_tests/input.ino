/* Functions to handle all input elements */

#include "mainSettings.h"

// Activate general trace output

#ifdef TRACE_ON
#define TRACE_INPUT 
#define TRACE_INPUT_ENCODER
#endif

/* Encoder constants and variables for IDS tracking function */

#define ENCODER_CLOCK_PIN 2
#define ENCODER_DIRECTION_PIN 4
#define ENCODER_MAX_CHANGE_PER_TICK 100

volatile bool encoder_prev_clock_state=false;
volatile bool encoder_direction_state_start=false;

volatile int encoder_change_value = 0;

/* encoder variables */
int input_encoder_value = 0;
int input_encoder_rangeMin = 0;
int input_encoder_rangeMax = 719;
int input_encoder_rangeShiftValue=input_encoder_rangeMax-input_encoder_rangeMin+1;
int input_encoder_stepSize = 1;
bool input_encoder_wrap = true;
bool input_encoder_change_event = false;



/* General state variables */
unsigned long input_last_event_time = 0;
bool input_enabled=true;

/* ***************************       S E T U P           ******************************
*/

void input_setup() {


  /* Initalize the encoder */
  input_setEncoderRange(1, 20, 1, 1); /* Set Encoder to count from 1 to 20  as default (number of ticks of my test hardware) */

  attachInterrupt(digitalPinToInterrupt(ENCODER_CLOCK_PIN),encoder_clock_change_ISR,CHANGE);

}

/* ********************************************************************************************************** */
/*               Interface functions                                                                          */

/* ------------- General Information --------------- */

int input_getSecondsSinceLastEvent() {
  unsigned long timestamp_difference = (millis() - input_last_event_time) / 1000;
  if (timestamp_difference > 255) return 255;
#ifdef TRACE_INPUT_TIMING
  Serial.print(F("TRACE_INPUT_TIMING:input last interaction:"));
  Serial.println(timestamp_difference);
#endif
  return timestamp_difference;
}

/* ------------- Encoder state --------------- */

bool input_hasEncoderChangePending() {
  return input_encoder_change_event;
}

int input_getEncoderValue() {
  input_encoder_change_event = false;
  return input_encoder_value;
}

/* ------------- Operations ----------------- */

void input_setEncoderValue(int newValue) {
  input_encoder_value = newValue;
  if (input_encoder_value < input_encoder_rangeMin) input_encoder_value = input_encoder_rangeMin;
  if (input_encoder_value > input_encoder_rangeMax) input_encoder_value = input_encoder_rangeMax;
  input_encoder_change_event = false;
}

void input_setEncoderRange(int rangeMin, int rangeMax, int stepSize, bool wrap) {
  input_encoder_rangeMin = min(rangeMin, rangeMax);
  input_encoder_rangeMax = max(rangeMin, rangeMax);
  input_encoder_rangeShiftValue=input_encoder_rangeMax-input_encoder_rangeMin+1;
  input_encoder_wrap = wrap;
  input_encoder_stepSize = stepSize;
  #ifdef TRACE_INPUT
    Serial.print(F("TRACE_INPUT input_setEncoderRange:"));
    Serial.print(rangeMin); Serial.print(F("-"));
    Serial.print(rangeMax); Serial.print(F(" Step "));
    Serial.print(stepSize); Serial.print(F(" Wrap "));
    Serial.println(wrap, BIN);
  #endif
}



/* *************************** internal implementation *************************** */

 
/* **** Encoder movement interrupt service routines, counts turing ticks into encoder_change_value 
*/

void encoder_clock_change_ISR()
{
  bool direction_state=digitalRead(ENCODER_DIRECTION_PIN);
  bool clock_state=digitalRead(ENCODER_CLOCK_PIN);  

  digitalWrite(LED_BUILTIN, clock_state);

  if(!encoder_prev_clock_state && clock_state) { //clock changes from 0 to 1
      encoder_direction_state_start=direction_state;  
      encoder_prev_clock_state=1;
      return;
  }

  if(encoder_prev_clock_state && !clock_state) { //clock changes from 1 to 0
      bool encoder_direction_state_end=direction_state;  
      encoder_prev_clock_state=0;
      if(encoder_direction_state_start && !encoder_direction_state_end) { // turned clockwise direction 
        encoder_change_value+=1;
      }
      if(!encoder_direction_state_start && encoder_direction_state_end) { // turned counter clockwise
       encoder_change_value-=1;
      }
      encoder_direction_state_start=encoder_direction_state_end;
  }
}      



/* ************************************* TICK ***********************************************
   translate the state of buttons into the ticks of the master loop
   Must be called by the master loop for every cycle to provide valid event states of
   all input devices.
   Also transfers state changes, tracked with the timer interrupt into a tick state
*/

void input_scan_tick()
{
  bool is_relevant_event=false;
  
  /* transfer high resolution encoder movement into tick encoder value */
  int tick_encoder_change_value = encoder_change_value;  // Freeze the value for upcoming operations
  if (tick_encoder_change_value) { // there are accumulated changes
    if(input_enabled && tick_encoder_change_value<ENCODER_MAX_CHANGE_PER_TICK && tick_encoder_change_value>-ENCODER_MAX_CHANGE_PER_TICK)   {
      input_encoder_value += tick_encoder_change_value * input_encoder_stepSize;
      // Wrap or limit the encoder value 
      while (input_encoder_value > input_encoder_rangeMax)      input_encoder_value = input_encoder_wrap ? input_encoder_value-input_encoder_rangeShiftValue : input_encoder_rangeMax;
      while (input_encoder_value < input_encoder_rangeMin) input_encoder_value = input_encoder_wrap ? input_encoder_value+input_encoder_rangeShiftValue : input_encoder_rangeMin;
      #ifdef TRACE_INPUT_ENCODER
        Serial.print(F("TRACE_INPUT_ENCODER input_switches_scan_tick:"));
        Serial.print(F(" tick_encoder_change_value=")); Serial.print(tick_encoder_change_value);
        Serial.print(F(" input_encoder_value=")); Serial.println(input_encoder_value);
      #endif
      input_encoder_change_event = true;
      is_relevant_event=true;
    }
    encoder_change_value -= tick_encoder_change_value; // remove the transfered value from the tracking
  }

  if(is_relevant_event)input_last_event_time = millis(); // Reset the global age of interaction

} // void input_switches_tick()





