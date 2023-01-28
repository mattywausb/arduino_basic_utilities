#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif



#include "mainSettings.h"
#include "Lamphsv.h"
#include "LampTransition.h"

#ifdef TRACE_ON
#define TRACE_OUTPUT
//#define TRACE_HSV_HIGH
//#define TRACE_OUTPUT_HIGH
#endif

#define PIXEL_DATA_PIN 12
 

#define PIXEL_COUNT 8
#define PIXEL_BRIGHTNESS 100 // only 1/2 of amps necessary

Adafruit_NeoPixel light_bar=Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_DATA_PIN, NEO_GRB + NEO_KHZ800);

LampTransition output_g_master_lamp;

t_lamp_rgb_color output_g_color_register_1={0,0,0};
t_lamp_rgb_color output_g_color_register_2={0,0,0};
t_lamp_rgb_color output_g_color_register_m={0,0,0};

byte output_g_counter=0;

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
    output_g_color_register_1.g=255;
    output_g_color_register_1.b=0;
    output_led_set_bar(&output_g_color_register_1);
    delay(500);
    output_g_color_register_1.g=0;
    output_led_set_bar(&output_g_color_register_1);
}



/* ============= SCENES ============== */


/* ---- SHOW ---- */

void output_init_SHOW_scene()
{
  output_g_color_register_1.r=
  output_g_color_register_1.g=
  output_g_color_register_1.b=0;
  output_g_master_lamp.setCurrentColor(&output_g_color_register_1);

  g_Lamphsv.get_color_rgb(&output_g_color_register_2);
  output_g_master_lamp.setTargetColor(&output_g_color_register_2);
  output_g_counter=0;
  output_led_set_bar(&output_g_color_register_1);
}


void output_update_SHOW_scene()
{
  LampTransition::setCurrentTime(millis());
  if(output_g_master_lamp.isTransitionPending()) output_g_master_lamp.startTransition(1000);
  if(output_g_master_lamp.isInTransition()) {
    output_g_master_lamp.getCurrentColor(&output_g_color_register_m);
    output_led_set_bar_range(output_g_counter, output_g_counter+2, &output_g_color_register_m);
    output_led_push();
  } else {
    if(++output_g_counter>=PIXEL_COUNT-2) output_g_counter=0;
    output_led_set_bar(&output_g_color_register_1);
    output_g_master_lamp.setCurrentColor(&output_g_color_register_1);
    output_g_master_lamp.setTargetColor(&output_g_color_register_2);
    output_led_push();
    output_g_master_lamp.startTransition(1000);
  }
  
}

/* ---- SET ---- */

void output_init_SET_scene()
{
  output_g_color_register_1.r=
  output_g_color_register_1.g=
  output_g_color_register_1.b=0;
  output_led_set_bar(&output_g_color_register_1);
  g_Lamphsv.get_color_rgb(&output_g_color_register_2);
  output_led_set_pixel(0,&output_g_color_register_2);
  output_led_push();
}

void output_update_SET_scene()
{
  if(!g_Lamphsv.is_changed()) return;

  g_Lamphsv.get_color_rgb(&output_g_color_register_2);
  output_led_set_pixel(0,&output_g_color_register_2);
  output_led_push();
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




