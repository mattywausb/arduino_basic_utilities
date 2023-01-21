#include "mainSettings.h"

#define KEY_COUNT 3
int press_counter[KEY_COUNT];
int encoder_button_press_count=0;
unsigned long prev_dump_time=0;
#define DUMP_INTERVAL 2000


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
} // end of setup()

void loop() {
  bool input_event_happened=false;

  input_scan_tick();

  // Enable/Disable encoder tracking  
  if(input_encoder_gotPressed()) {
      encoder_button_press_count++;
      if(encoder_button_press_count%2) input_encoder_disable();
      else input_encoder_enable();
  }

  // Evaluate keyboard input
  for(byte k=0;k<KEY_COUNT;k++) {
    if(input_keyGotPressed( k)) {
      press_counter[k]++;
      input_event_happened=true;
    }
  }

  //Print new value when changed
  if(input_event_happened) {
    Serial.print("KEY COUNTS ");
    for(byte k=0;k<KEY_COUNT;k++) {
      Serial.print(press_counter[k]);
      Serial.print("  ");
    }
    Serial.println();
  }
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

} // end of loop()
