#include "mainSettings.h"

#define KEY_COUNT 3
int press_counter[KEY_COUNT];
int encoder_button_press_count=0;


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
  // put your main code here, to run repeatedly:
  input_scan_tick();
  bool input_event_happened=false;

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

} // end of loop()
