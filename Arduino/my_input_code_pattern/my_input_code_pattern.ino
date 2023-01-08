#include "mainSettings.h"

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

  // finally
  digitalWrite(LED_BUILTIN, false); // setup complete, so switch off LED
}

void loop() {
  // put your main code here, to run repeatedly:
  input_scan_tick();
}
