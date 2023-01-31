#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif



#include "mainSettings.h"
#include "Lamphsv.h"
#include "LampTransition.h"

#ifdef TRACE_ON
#define TRACE_OUTPUT
//#define TRACE_OUTPUT_RGB
#endif

#define PIXEL_DATA_PIN 12
 

#define PIXEL_COUNT 8
#define PIXEL_BRIGHTNESS 100 // only 1/2 of amps necessary

Adafruit_NeoPixel light_bar=Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);

LampTransition output_g_transition[PIXEL_COUNT];  // For indivudal transition of all lamps

t_lamp_rgb_color output_g_color_register_1={0,0,0};
t_lamp_rgb_color output_g_color_register_2={0,0,0};
t_lamp_rgb_color output_g_color_register_m={0,0,0};




byte output_g_counter=0;
bool output_g_calibrated=true;
Lamphsv output_hand_lamp;



/* ***************************       S E T U P           ******************************
   Must be called by setup function of sketch to initialize all output ports, 
   tracking variables and devices
 * ----------------------------------------------------------------------------------- */

void output_setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, false);                        
    
    light_bar.begin(); 
    light_bar.setBrightness(PIXEL_BRIGHTNESS);  
    output_g_color_register_1.r=0;
    output_g_color_register_1.g=128;
    output_g_color_register_1.b=0;
    output_led_set_bar(&output_g_color_register_1);
    output_led_push();
    delay(500);
    output_g_color_register_1.g=0;
    output_led_set_bar(&output_g_color_register_1);
    output_led_push();
    Lamphsv::setCalibrationHueAngle (LAMPHSV_CALIBRATION_INDEX_YELLOW,48);
    Lamphsv::setCalibrationHueAngle (LAMPHSV_CALIBRATION_INDEX_CYAN,194);
    Lamphsv::setCalibrationHueAngle (LAMPHSV_CALIBRATION_INDEX_MAGENTA,257);

   
}

/* ---------- configuration ------------- */

void output_flipCalibration()
{
  if(Lamphsv::getCalibrationState())  Lamphsv::disableCalibration();
  else  Lamphsv::enableCalibration();
  g_Lamphsv.get_color_rgb(&output_g_color_register_2);
  #ifdef TRACE_OUTPUT
      Serial.print(F("TRACE_OUTPUT> calibration="));
      Serial.println(Lamphsv::getCalibrationState());
  #endif
}



/* ============= SCENES ============== */


/* ---- SHOW ---- */

void output_init_SHOW_scene()
{
  output_g_counter=0;

  output_g_color_register_1.r=
  output_g_color_register_1.g=
  output_g_color_register_1.b=0;
  for(byte p=0;p<PIXEL_COUNT;p++)  output_g_transition[p].setCurrentColor(&output_g_color_register_1); // turn all lamps black

  g_Lamphsv.get_color_rgb(&output_g_color_register_2); // Color of the calibrated hsv is our intermediate color
  output_g_transition[output_g_counter].setTargetColor(&output_g_color_register_2); // trigger first lamp to transition to current hsv color
  output_g_transition[output_g_counter].startTransition(1000);

  // initialize bar to black
  output_led_set_bar(&output_g_color_register_1); 
  output_led_push();
  
  // dark pastell cyan is our background color in the upcoming animation
  Lamphsv lamphsv_background;
  lamphsv_background.set_hsv(240,90,15);
  lamphsv_background.get_color_rgb(&output_g_color_register_1);

  output_hand_lamp.set_saturation(100);
  output_hand_lamp.set_value(90);

}

void output_update_hand_hue() 
{
  int circle_position_in_hour=359-((millis()/1000)%3600)/10;  // Determine hue of time
  output_hand_lamp.set_hue(circle_position_in_hour-g_hue_to_time_offset); // shift hue by offset and set it
  output_hand_lamp.get_color_rgb(&output_g_color_register_2);
  output_g_transition[output_g_counter].setTargetColor(&output_g_color_register_2); // trigger next lamp to transition
}

void output_update_SHOW_scene()
{
  LampTransition::setCurrentTime(millis());

  // Trigger lamp transitions as needed 

  if(!output_g_transition[output_g_counter].isInTransition()) { // 1st part of transition of current focussed lamp is complete
       output_g_transition[output_g_counter].setTargetColor(&output_g_color_register_1); // initiate fade of current lamp
       output_g_transition[output_g_counter].startTransition(30000);
      
      if(++output_g_counter>=PIXEL_COUNT) output_g_counter=0;  // switch to next lamp
      output_update_hand_hue(); 
      output_g_transition[output_g_counter].startTransition(7500);
  }
  

  // update all pixels that currently have a transition
  for(byte p=0; p<PIXEL_COUNT;p++) {  
     if(output_g_transition[p].isInTransition()) {
       #ifdef TRACE_LAMPTRANSITION_MODULATION
          Serial.print(F("p "));Serial.print(p);
        #endif
        output_g_transition[p].getModulatedColor(&output_g_color_register_m);
        output_led_set_pixel(p, &output_g_color_register_m);
        #ifdef TRACE_OUTPUT_RGB
          output_dumpRgbToSerial(&output_g_color_register_m);
        #endif  
     }
  }
  output_led_push();
}

/* ---- SET ---- */

void output_init_SET_scene()
{
  output_g_color_register_1.r=
  output_g_color_register_1.g=
  output_g_color_register_1.b=0;
  output_led_set_bar(&output_g_color_register_1);
  g_Lamphsv.get_color_rgb(&output_g_color_register_2);
  output_led_set_pixel(g_set_pixel,&output_g_color_register_2);
  output_led_push();
}

void output_update_SET_scene_switch_pixel()
{
  g_Lamphsv.get_color_rgb(&output_g_color_register_2);
  output_led_set_pixel(g_set_pixel,&output_g_color_register_2);
  output_led_push();
}


void output_update_SET_scene()
{
  if(!g_Lamphsv.is_changed()) return;
  
  g_Lamphsv.get_color_rgb(&output_g_color_register_2);
  output_led_set_pixel(g_set_pixel,&output_g_color_register_2);
  output_led_push();
  #ifdef TRACE_OUTPUT_RGB
     Serial.print(F("TRACE_OUTPUT_RGB>"));
     output_dumpRgbToSerial(&output_g_color_register_2);
  #endif   

}



/* ======== Utilities =========== */

/* set one pixel on the led  manually  */

void output_led_set_pixel(byte pixel_index, t_lamp_rgb_color*  pColor)
{
  light_bar.setPixelColor(pixel_index, light_bar.Color(pColor->r,pColor->g,pColor->b));
}


/* set pixel range on the led   */
void output_led_set_bar_range(byte first_index,byte last_index, t_lamp_rgb_color*  pColor)
{
  if(last_index >= PIXEL_COUNT) last_index=PIXEL_COUNT-1;
  if(first_index>last_index) first_index=last_index;
  for(byte p=first_index;p<=last_index;p++) {
    light_bar.setPixelColor(p, light_bar.Color(pColor->r,pColor->g,pColor->b));
  }
};


/* set all pixel on the led   */ 
void output_led_set_bar (t_lamp_rgb_color*  pColor)
{
  for(byte p=0;p<PIXEL_COUNT;p++) {
    light_bar.setPixelColor(p, light_bar.Color(pColor->r,pColor->g,pColor->b));
  };
} ;

void output_led_push()
{
  #ifdef TRACE_OUTPUT_HIGH
      Serial.println(F(">output_show"));
  #endif
    light_bar.show();                                  
}

/* **************** Trace routines ********************** */

#ifdef TRACE_OUTPUT_RGB
void output_dumpRgbToSerial(t_lamp_rgb_color *pRgb)
{
    Serial.print("(");Serial.print(pRgb->r);
    Serial.print(",");Serial.print(pRgb->g);
    Serial.print(",");Serial.print(pRgb->b);
    Serial.println(")");
}
#endif




