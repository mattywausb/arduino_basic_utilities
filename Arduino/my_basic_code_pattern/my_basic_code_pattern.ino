#include "mainSettings.h"
#include "Lamphsv.h"

#ifdef TRACE_ON
  #define TRACE_MODES
  #define TRACE_PARAMETER_CHANGE
#endif  

/* ------- Mode Control variables ---------- */
enum PROCESS_MODES {
  SHOW, 
  SET_HUE,
  SET_SATURATION
};

Lamphsv g_Lamphsv; // central Lamphsv parameters that will be changed by input


PROCESS_MODES g_process_mode=SHOW;

/* -------- Application variables and constants ------- */

#define KEY_COUNT 3
int press_counter[KEY_COUNT];
int encoder_button_press_count=0;
unsigned long prev_dump_time=0;
#define DUMP_INTERVAL 1000



/* **************** SETUP ************************** */
void setup() {
  // put your setup code here, to run once:
  #ifdef TRACE_ON 
    char compile_signature[] = "--- START (Build: " __DATE__ " " __TIME__ ") ---";   
    Serial.begin(9600);
    Serial.println(compile_signature); 
  #endif

  // Init all Pins and interfaces
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, true); // light LED during setup

  input_setup();  // first initialize the inputs, in case the output setup takes direct values from it
  output_setup();

  // init variables
  for(byte k=0;k< KEY_COUNT;k++) {
    press_counter[k]=0;
  }

  g_Lamphsv.set_hsv(0,100,100);

  // finally
  digitalWrite(LED_BUILTIN, false); // setup complete, so switch off LED
  output_init_SHOW_scene();
  enter_mode_SHOW();
} // end of setup()

/* **************** LOOP ************************** */

void loop() 
{
   input_scan();

   switch(g_process_mode) {
    case SHOW: process_mode_SHOW();break;
    case SET_HUE: process_mode_SET_HUE();break;
   } // switch
}

/* ========= MODE SHOW =================== */

void enter_mode_SHOW() 
{
    g_process_mode=SHOW;
    input_ignoreUntilRelease(true);
    #ifdef TRACE_MODES
      Serial.println(F("---> SHOW <---"));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
    #endif  
    output_init_SHOW_scene();
}

void process_mode_SHOW() {
  bool input_event_happened=false;

  if(input_isRelevant()) {
    // Change to set mode in long press of key 1
    if(input_key_gotPressed(2)) {
      enter_mode_SET_HUE();
      return;
    }

    // Enable/Disable encoder tracking  (just for demo)
    if(input_encoder_gotPressed()) {
        encoder_button_press_count++;
        if(encoder_button_press_count%2) input_encoder_disable();
        else input_encoder_enable();
    }





    // Count keyboard presses (just for demo)
    for(byte k=0;k<KEY_COUNT;k++) {
      if(input_key_gotPressed( k)) {
        press_counter[k]++;
        input_event_happened=true;
      }
    }
  }

  //Print new value when some input changed
  if(input_event_happened) {
    Serial.print("KEY COUNTS ");
    for(byte k=0;k<KEY_COUNT;k++) {
      Serial.print(press_counter[k]);
      Serial.print("  ");
    }
    Serial.println();
  }

  output_update_SHOW_scene();

  // Dump keyboard values to serial everey DUMP_INTERVAL
  unsigned long current_time = millis();
  if(current_time-prev_dump_time>=DUMP_INTERVAL) {
    prev_dump_time=current_time;
    Serial.print("Processor Time ");
    uint16_t current_time=millis();
    Serial.print(current_time);
    Serial.print(" Encoder Value ");
    Serial.print(input_encoder_getValue());
    Serial.print(" Press Timer ");
    for(byte k=0;k<KEY_COUNT;k++) {
      Serial.print(input_key_getPressDuration( k));
      Serial.print(":");
      Serial.print(input_key_getReleaseDuration( k));
      Serial.print("   ");

    }
    Serial.println();
  }
} 

/* ========= MODE SET_HUE =================== */

void enter_mode_SET_HUE() {

  g_process_mode=SET_HUE;
  input_ignoreUntilRelease(true);
  #ifdef TRACE_MODES
      Serial.println(F("---> SET_HUE <---"));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
  #endif  
  input_encoder_setRange(0,359,true);
  input_encoder_setValue(g_Lamphsv.get_hue());
  output_init_SET_scene();
}

void process_mode_SET_HUE() {

  if(input_isRelevant()) {
    // Change to show mode when key 2 got pressed
    if(input_key_gotPressed(2)) {
      enter_mode_SHOW();
      return;
    }
    if(input_encoder_hasPendingChange()) {
      g_Lamphsv.set_hue(input_encoder_getValue());
      #ifdef TRACE_PARAMETER_CHANGE
        Serial.print(F("TRACE_PARAMETER_CHANGE > hue:"));
        Serial.println(g_Lamphsv.get_hue());
        g_Lamphsv.print_members_to_serial();
      #endif
    }
  } //end "if input is valid"

  output_update_SET_scene(); // out
}

/* ******************** Memory Helper *************** */
 
#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char *__brkval;
#endif  // __arm__

int freeMemory() {
  char top;
#ifdef __arm__
  return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
  return &top - __brkval;
#else  // __arm__
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}
