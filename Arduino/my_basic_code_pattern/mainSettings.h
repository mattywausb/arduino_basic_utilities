/*
  This header file is sourced by all other files and is used to provide some major 
  configuration switches for debugging and tracing.
  Release versions of the code should have switched off all tracing bei commenting the #define TRACE_ON instruction
*/

/* Master Switch for tracing events to serial (probably also if serial object will be created)*/
 #define TRACE_ON

 /* Master Switch for feedback of input signals on BUILT IN LED (might be left on in release) */
 #define INPUT_FEEDBACK_ON_LED_BUILTIN