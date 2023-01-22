#include "mainSettings.h"

#ifdef TRACE_ON
  #define TRACE_MODES
#endif  

/* ------- Mode Control variables ---------- */
enum PROCESS_MODES {
  SHOW, 
  SET
};

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

  input_setup();

  // init variables
  for(byte k=0;k< KEY_COUNT;k++) {
    press_counter[k]=0;
  }

  // finally
  digitalWrite(LED_BUILTIN, false); // setup complete, so switch off LED
  enter_mode_SHOW();
} // end of setup()

/* **************** LOOP ************************** */

void loop() 
{
   input_scan_tick();

   switch(g_process_mode) {
    case SHOW: process_mode_SHOW();break;
    case SET: process_mode_SET();break;
   } // switch
}

/* ========= MODE SHOW =================== */

void enter_mode_SHOW() 
{
    g_process_mode=SHOW;
    #ifdef TRACE_MODES
      Serial.println(F("---> SHOW <---"));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
    #endif  
}

void process_mode_SHOW() {
  bool input_event_happened=false;

  // Enable/Disable encoder tracking  
  if(input_encoder_gotPressed()) {
      encoder_button_press_count++;
      if(encoder_button_press_count%2) input_encoder_disable();
      else input_encoder_enable();
  }

  // Count keyboard presses
  for(byte k=0;k<KEY_COUNT;k++) {
    if(input_keyGotPressed( k)) {
      press_counter[k]++;
      input_event_happened=true;
    }
  }

  // Change to set mode after release from long press of key 0
  if(input_keyGotReleased(0) ) {
    if(input_keyGetPressDuration(0)>2000  ) {
      enter_mode_SET();
      return;
    }
    Serial.print("Press Duration 0 = ");      Serial.println(input_keyGetPressDuration(0));
  }

  // Change to set mode in long press of key 1
  if(input_keyIsPressed(1) && input_keyGetPressDuration(1)>2000) {
    enter_mode_SET();
    return;
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

  // Dump keyboard values to serial everey DUMP_INTERVAL
  unsigned long current_time = millis();
  if(current_time-prev_dump_time>=DUMP_INTERVAL) {
    prev_dump_time=current_time;
    Serial.print("Processor Time ");
    uint16_t current_time=millis();
    Serial.print(current_time);
    Serial.print(" Press Timer ");
    for(byte k=0;k<KEY_COUNT;k++) {
      Serial.print(input_keyGetPressDuration( k));
      Serial.print(":");
      Serial.print(input_keyGetReleaseDuration( k));
      Serial.print("   ");

    }
    Serial.println();
  }
} 

/* ========= MODE SET =================== */

void enter_mode_SET() {

  g_process_mode=SET;
  input_ignore_until_release(true);
  #ifdef TRACE_MODES
      Serial.println(F("---> SET <---"));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
  #endif  
}

void process_mode_SET() {

  // Change to show mode when key 2 got pressed
  if(input_is_valid()) {
    if(input_keyGotPressed(2)) {
        enter_mode_SHOW();
        return;
    }
  } //end "if input is valid"
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
