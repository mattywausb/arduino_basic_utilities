/* Functions to handle all input elements */

#include "mainSettings.h"

// Activate general trace output

#ifdef TRACE_ON
#define TRACE_INPUT 
#define TRACE_INPUT_ENCODER
#endif

/******** Encoder configuration  ********/

// CLOCK PIN must be handled by interrupt
#define ENCODER_CLOCK_PIN 2
#define ENCODER_DIRECTION_PIN 4
#define ENCODER_MAX_CHANGE_PER_TICK 100

// Press function is managed as normal switch

/********* Switch configuration *********/
#define ENCODER_SWITCH_PIN 5
byte keyboard_pin[]={6,7,8};
#define KEYBOARD_BUTTON_COUNT 3

/* *********** General state variables of the input module ************ */
unsigned long input_last_event_time = 0;
bool input_enabled=true;

/******** Encoder constants and variables  ********/

volatile bool encoder_prev_clock_state=HIGH;
volatile bool encoder_direction_state_start=HIGH;
volatile int8_t encoder_change_value = 0;
#ifdef TRACE_INPUT_ENCODER
  volatile byte isr_call_count=0;
#endif 

int input_encoder_value = 0;
int input_encoder_rangeMin = 0;
int input_encoder_rangeMax = 719;
int input_encoder_rangeShiftValue=input_encoder_rangeMax-input_encoder_rangeMin+1;
int input_encoder_stepSize = 1;
bool input_encoder_wrap = true;
bool input_encoder_change_event = false;

/****** switch constants and variables ******/



/* ***************************       S E T U P           ******************************
*/

void input_setup() {


  /* Initalize the encoder */
  input_encoder_setRange(1, 20, 1, true); // Set Encoder to count from 1 to 20  as default (number of on turns in my test hardware) 
  attachInterrupt(digitalPinToInterrupt(ENCODER_CLOCK_PIN),encoder_clock_change_ISR,CHANGE);

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

/* ******************* General scan function ********************
* call all scan functions and keeps track of last_event timestamp
*/

void input_scan_tick()
{
  if(input_encoder_scan() /*|| input_switch_scan()*/) input_last_event_time = millis(); // Reset the global age of interaction

} // input_scan_tick


/* ***************** Encoder state functions ************************* */

bool input_encoder_hasPendingChange() {  
  return input_encoder_change_event;
}

int input_encoder_getValue() {
  input_encoder_change_event = false;
  return input_encoder_value;
}

/* **************** Encoder  Operations ***************** */

/* Set the current value of the encoder. Will be placed into the valid range, when out of bounds */
void input_encoder_setValue(int newValue) {
  input_encoder_value = newValue;
  if (input_encoder_value < input_encoder_rangeMin) input_encoder_value = input_encoder_rangeMin;
  if (input_encoder_value > input_encoder_rangeMax) input_encoder_value = input_encoder_rangeMax;
  input_encoder_change_event = false;
}

/* Set the value range and step size for the encoder, including the option to wrap or not */
void input_encoder_setRange(int rangeMin, int rangeMax, int stepSize, bool wrap) {
  input_encoder_rangeMin = min(rangeMin, rangeMax);
  input_encoder_rangeMax = max(rangeMin, rangeMax);
  input_encoder_rangeShiftValue=input_encoder_rangeMax-input_encoder_rangeMin+1;
  input_encoder_wrap = wrap;
  input_encoder_stepSize = stepSize;
  input_encoder_setValue(input_encoder_value);
  #ifdef TRACE_INPUT
    Serial.print(F("TRACE_INPUT input_encoder_setRange:"));
    Serial.print(rangeMin); Serial.print(F("-"));
    Serial.print(rangeMax); Serial.print(F(" Step "));
    Serial.print(stepSize); Serial.print(F(" Wrap "));
    Serial.println(wrap, BIN);
  #endif
}

/* **************** Encoder  Scan functions ***************** */

/* This must be called by the loop to update the encoder state */
bool input_encoder_scan()
{
  bool is_relevant_event=false;
  
  /* transfer high resolution encoder movement into tick encoder value */
  int8_t tick_encoder_change_value = encoder_change_value;  // Freeze the value for upcoming operations
  if (tick_encoder_change_value) { // there are accumulated changes
    if(input_enabled && tick_encoder_change_value<ENCODER_MAX_CHANGE_PER_TICK && tick_encoder_change_value>-ENCODER_MAX_CHANGE_PER_TICK)   {
      input_encoder_value += tick_encoder_change_value * input_encoder_stepSize;
      // Wrap or limit the encoder value 
      while (input_encoder_value > input_encoder_rangeMax) input_encoder_value = input_encoder_wrap ? input_encoder_value-input_encoder_rangeShiftValue : input_encoder_rangeMax;
      while (input_encoder_value < input_encoder_rangeMin) input_encoder_value = input_encoder_wrap ? input_encoder_value+input_encoder_rangeShiftValue : input_encoder_rangeMin;

      input_encoder_change_event = true;
      is_relevant_event=true;
    }
    encoder_change_value -= tick_encoder_change_value; // remove the transfered value from the tracking
    #ifdef TRACE_INPUT_ENCODER
        Serial.print(F("TRACE_INPUT_ENCODER input_switches_scan_tick:"));
        Serial.print(F(" isr_call_count=")); Serial.print(isr_call_count);
        Serial.print(F("\ttick_encoder_change_value=")); Serial.print(tick_encoder_change_value);
        Serial.print(F("\tinput_encoder_value=")); Serial.print(input_encoder_value);
        Serial.print(F("\tencoder_change_value left=")); Serial.println(encoder_change_value);
        isr_call_count=0;
    #endif
  }

  return is_relevant_event;

}

/* This is the encoders interrupt function */
// Todo: provide option to use PULL UP or PULL DOWN version
void encoder_clock_change_ISR()
{
  bool direction_state=digitalRead(ENCODER_DIRECTION_PIN);
  bool clock_state=digitalRead(ENCODER_CLOCK_PIN);  

  #ifdef TRACE_INPUT_ENCODER
    isr_call_count++;
  #endif
  #ifdef INPUT_FEEDBACK_ON_LED_BUILTIN
    digitalWrite(LED_BUILTIN, !clock_state);
  #endif

  if(encoder_prev_clock_state && !clock_state) { //clock changes from 1 to 0 (start of cycle in PULL UP logic)
      encoder_direction_state_start=direction_state;  
      encoder_prev_clock_state=clock_state;
      return;
  }

  if(!encoder_prev_clock_state && clock_state) { //clock changes from 1 to 0 (end of cycle PULL UP logic)
      bool encoder_direction_state_end=direction_state;  
      encoder_prev_clock_state=clock_state;
      if(!encoder_direction_state_start) {
        if (encoder_direction_state_end) { // turned counter clockwise 
          encoder_change_value-=1;
        }
      } else { 
        if(!encoder_direction_state_end) { // turned  clockwise
          encoder_change_value+=1;
        }
      }
      encoder_direction_state_start=encoder_direction_state_end;
  }
}      








