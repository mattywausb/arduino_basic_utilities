#include "mainSettings.h"
#include "Lamphsv.h"

#ifdef TRACE_ON
  #define TRACE_MODES
  #define TRACE_PARAMETER_CHANGE
  //#define TRACE_KEYBOARD_STATISTIC
  #define TRACE_LOOP_THROUGHPUT
#endif  

/* ------- Mode Control variables ---------- */
enum PROCESS_MODES {
  SHOW, 
  SET_HUE,
  SET_SATURATION,
  SET_VALUE
};

Lamphsv g_Lamphsv; // central Lamphsv parameters that will be changed by input


byte g_step_size=1;
byte g_set_pixel=0;
int g_hue_to_time_offset=0;

#ifdef TRACE_LOOP_THROUGHPUT
uint32_t loop_cumulated_runtime_us=0;
uint32_t loop_count=0;
uint32_t loop_longest_runtime=0;
#endif


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
   #ifdef TRACE_LOOP_THROUGHPUT
   loop_count++;
   uint32_t loop_start_time_us=micros();
   #endif

   input_scan();

   switch(g_process_mode) {
    case SHOW: process_mode_SHOW();break;
    case SET_HUE: process_mode_SET_HUE();break;
    case SET_SATURATION: process_mode_SET_SATURATION();break;
    case SET_VALUE: process_mode_SET_VALUE();break;
   } // switch
   
   #ifdef TRACE_LOOP_THROUGHPUT
   uint32_t loop_runtime=micros()-loop_start_time_us;
   loop_cumulated_runtime_us+=loop_runtime;
   loop_longest_runtime=max(loop_runtime,loop_longest_runtime);
   if(loop_cumulated_runtime_us>10000000) {
     uint16_t loop_average_runtime_us=loop_cumulated_runtime_us/loop_count;
     Serial.print(F("TRACE_LOOP_THROUGHPUT> loop_count="));Serial.print(loop_count);
     Serial.print(F(" loop_cumulated_runtime_us="));Serial.print(loop_cumulated_runtime_us);
     Serial.print(F(" loop_longest_runtime="));Serial.print(loop_longest_runtime);
     Serial.print(F(" loop_average_runtime_us="));Serial.print(loop_average_runtime_us);
     Serial.print(F(" fps="));Serial.println(1000000/loop_average_runtime_us);
     loop_cumulated_runtime_us=0;
     loop_longest_runtime=0;
     loop_count=0;
   }
   #endif

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


   input_encoder_setRange(0,359,6,true); // Step of 6 = 60 Stepa for 1 circle = 1 Step 1 minute
   input_encoder_setValue(g_hue_to_time_offset);
   output_init_SHOW_scene();
}

void process_mode_SHOW() {
  bool input_event_happened=false;

  if(input_isRelevant()) {
    // Change to set mode in long press of key 1
    if(input_key_gotPressed(2)) {
      output_init_SET_scene();
      enter_mode_SET_HUE();
      return;
    }

    // Change calibration mode on key 0
    if(input_key_gotPressed(0)) {
      output_flipCalibration();
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

    // tranfer encoder value to state variable
    if(input_encoder_hasPendingChange()) {
      g_hue_to_time_offset=input_encoder_getValue();
      output_update_hand_hue() ;
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

  #ifdef TRACE_KEYBOARD_STATISTIC
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
  #endif
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
  g_step_size=15;
  input_encoder_setRange(0,359,g_step_size,true);
  input_encoder_setValue(g_Lamphsv.get_hue());
}

void process_mode_SET_HUE() {

  if(input_isRelevant()) {
    // Change to show mode when key 2 got pressed
    if(input_key_gotPressed(2)) {
      enter_mode_SHOW();
      return;
    }

    // Change to set saturation mode when key 1 got pressed

    if(input_key_gotPressed(1)) {
      enter_mode_SET_SATURATION();
      return;
    }

    // Switch to next pixel and flip calibration when key 0 got pressed
    if(input_key_gotPressed(0)) {
      if(++g_set_pixel>=8)g_set_pixel=0;      
      output_flipCalibration();
      output_update_SET_scene_switch_pixel();
    }
    

    // Change step size of encoder
    if(input_encoder_gotPressed()) {
        g_step_size=g_step_size>1?1:15;
        input_encoder_setRange(0,359,g_step_size,true);
    }

    // Set hue to encoder value 
    if(input_encoder_hasPendingChange()) {
      g_Lamphsv.set_hue(input_encoder_getValue());
      #ifdef TRACE_PARAMETER_CHANGE
        Serial.print(F("TRACE_PARAMETER_CHANGE > hue:"));
        Serial.println(g_Lamphsv.get_hue());
        g_Lamphsv.print_members_to_serial();
      #endif
    }
  } //end "if input_isRelevant"

  output_update_SET_scene(); // out
}

/* ========= MODE SET_SATURATION =================== */

void enter_mode_SET_SATURATION() {

  g_process_mode=SET_SATURATION;
  input_ignoreUntilRelease(true);
  #ifdef TRACE_MODES
      Serial.println(F("---> SET_SATURATION <---"));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
  #endif  
  g_step_size=5;
  input_encoder_setRange(0,100,g_step_size,false);
  input_encoder_setValue(g_Lamphsv.get_saturation());
}

void process_mode_SET_SATURATION() {

  if(input_isRelevant()) {
    // Change to show mode when key 2 got pressed
    if(input_key_gotPressed(2)) {
      enter_mode_SHOW();
      return;
    }

    // Change to set hue  mode when key 1 got pressed
    if(input_key_gotPressed(1)) {
      enter_mode_SET_VALUE();
      return;
    }

    // Switch to next pixel and flip calibration when key 0 got pressed
    if(input_key_gotPressed(0)) {
      if(++g_set_pixel>=8)g_set_pixel=0;
      output_flipCalibration();
      output_update_SET_scene_switch_pixel();
    }


    // Change step size of encoder
    if(input_encoder_gotPressed()) {
        g_step_size=g_step_size>1?1:5;
        input_encoder_setRange(0,100,g_step_size,false);
    }

    // Set hue to encoder value 
    if(input_encoder_hasPendingChange()) {
      g_Lamphsv.set_saturation(input_encoder_getValue());
      #ifdef TRACE_PARAMETER_CHANGE
        Serial.print(F("TRACE_PARAMETER_CHANGE > saturation:"));
        Serial.println(g_Lamphsv.get_saturation());
        g_Lamphsv.print_members_to_serial();
      #endif
    }
  } //end "if input_isRelevant"

  output_update_SET_scene(); 
}


/* ========= MODE SET_VALUE =================== */

void enter_mode_SET_VALUE() {

  g_process_mode=SET_VALUE;
  input_ignoreUntilRelease(true);
  #ifdef TRACE_MODES
      Serial.println(F("---> SET_VALUE <---"));
      Serial.print(freeMemory());
      Serial.print(F(" bytes free memory. "));
      Serial.print(millis()/1000);
      Serial.println(F(" seconds uptime"));
  #endif  
  g_step_size=5;
  input_encoder_setRange(0,100,g_step_size,false);
  input_encoder_setValue(g_Lamphsv.get_value());
}

void process_mode_SET_VALUE() {

  if(input_isRelevant()) {
    // Change to show mode when key 2 got pressed
    if(input_key_gotPressed(2)) {
      enter_mode_SHOW();
      return;
    }

    // Change to set hue  mode when key 1 got pressed
    if(input_key_gotPressed(1)) {
      enter_mode_SET_HUE();
      return;
    }

    // Switch to next pixel and flip calibration when key 0 got pressed
    if(input_key_gotPressed(0)) {
      if(++g_set_pixel>=8)g_set_pixel=0;
      output_flipCalibration();
      output_update_SET_scene_switch_pixel();
    }


    // Change step size of encoder
    if(input_encoder_gotPressed()) {
        g_step_size=g_step_size>1?1:5;
        input_encoder_setRange(0,100,g_step_size,false);
    }

    // Set hue to encoder value 
    if(input_encoder_hasPendingChange()) {
      g_Lamphsv.set_value(input_encoder_getValue());
      #ifdef TRACE_PARAMETER_CHANGE
        Serial.print(F("TRACE_PARAMETER_CHANGE > value:"));
        Serial.println(g_Lamphsv.get_value());
        g_Lamphsv.print_members_to_serial();
      #endif
    }
  } //end "if input_isRelevant"

  output_update_SET_scene(); 
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
