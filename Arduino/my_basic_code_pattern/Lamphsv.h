#ifndef LAMPHSV_H
#define LAMPHSV_H
#include "Arduino.h"
#include "mainSettings.h"

#define LAMPHSV_HUE_RED 0
#define LAMPHSV_HUE_ORANGE 15.0
#define LAMPHSV_HUE_ORANGE 15.0
#define LAMPHSV_HUE_YELLOW 45.0
#define LAMPHSV_HUE_GREEN 120.0
#define LAMPHSV_HUE_CYAN 180.0
#define LAMPHSV_HUE_SKYBLUE 220.0
#define LAMPHSV_HUE_BLUE 240.0
#define LAMPHSV_HUE_PURPLE 250.0
#define LAMPHSV_HUE_MAGENTA 300.0
#define LAMPHSV_HUE_PINK 340.0


typedef struct {
    byte r;       // 0-255
    byte g;       // 0-255
    byte b;       // 0-255
} t_lamphsv_color_rgb;

typedef struct {
    int16_t h;       // angle in 1/10 degrees (0-3600)
    int8_t s;       // saturation from 0-100
    int8_t v;       // value from 0-100
} t_lamphsv_color;

/*!
  Store and manipulate color of a lamp as hue(in degree), saturation and value(=intensity). Provide RGB value for sending to 
  displays. Implementation with fast integer arithmetic and low memory footprint. (4 Bytes per lamp)
*/

class Lamphsv {
 public:
        /*!
          Constructor
        */
        Lamp(void);

        /*! 
          Set hsv values of the lamp
          @param hue Define the hue in 1/10s degrees (from 0 to 3599)
          @param saturation Define the saturation with 100  = full color and 0 = no color/white
          @param value Define the value with 100 = full brightness and 0= dark/off
        */
        void set_hsv(int16_t hue,int8_t saturation,int8_t value);

        /*! 
          Set hsv values of the lamp with float variable (Slowest version, due to overhead for float converion
          it is not recommende to use this in time critical high frequency loops)
          @param hue Define the hue in degrees (Only Fractions of 1/10 will be stored)
          @param saturation Define the saturation with 1.0 = full color and 0.0= not saturated/white
          @param value Define the value with 1.0 = full brightness and 0= dark/off
        */
        void set_hsv(float hue,float saturation,float value) {set_hsv((int16_t)(hue*10.0),(int8_t)(saturation*100.0),(int8_t)(value*100.0));};

        /*! 
          Get hue value 
          @return hue value in 1/10 degress (0-3599)
        */
        int16_t get_hue() {return m_hue};

        /*! 
          Get hue value as float
          @return hue value in degress (0-360)
        */
        int16_t get_hue_float() {return ((float)m_hue)/10.0);};
 
        /*! 
          Set hue to  a specific angle. Values out of bound will be set to 0.
          @param hue Define the hue in 1/10s degrees (from 0 to 3599)
        */
        void set_hue(int16_t angle) ;
        
        /*! 
          Set hue to  a specific angle. Values out of bound will be set to 0. (Slow float version)
          @param hue Define the hue in degrees (Only Fractions of 1/10 will be stored)
        */
        void set_hue(float angle) {set_hue((int16_t)(angle*10.0))};

        /*! 
          Change the hue by a specific angle. This will shift the color in the spectrum. 
          The result will be wrapped at the end of the value range to provide borderles behaviour
          @param angle angle in 1/10 degrees (can be negative)
        */
        void add_hue_angle(int16_t angle) ;

        /*! 
          Get saturation value 
          @return saturation value 100=full color 0= no color/white
        */
        int8_t get_saturation() {return m_saturation;}  ;

        /*! 
          Set saturation to  a specific value. 100=full color 0= no color/white.  Values out of bound will be clipped to lower or upper bound accordingly
          @param saturation 
        */
        void set_saturation(int8_t saturation)  ;

        /*! 
          Change saturation by an absolute increment. Resulting value will be clipped to lower or upper bound accordingly
          @param increment 
        */
        void add_saturation(int8_t increment);

        /*! 
          Change saturation by multiplying the current value by the given factor. 
          Resulting value will be clipped to lower or upper bound accordingly.
          This will provide fast slower blending on lower values
          @param factor in 1/100 : 100 = 1(no change) , 10= 0.10  1=0.01, 200=double, 300=triple
        */
        void multiply_saturation(int16_t factor);

        /*! 
          Get value 
          @return value value 0=dark 100= highest intensity
        */
        int8_t get_value() {return m_value;}  ;

        /*! 
          Set value to  a specific value. 0=dark 100= highest intensity  Values out of bound will be clipped to lower or upper bound accordingly
          @param value 
        */
        void set_value(int8_t value)  ;

        /*! 
          Change value by an absolute increment. Resulting value will be clipped to lower or upper bound accordingly
          @param increment 
        */
        void add_value(int8_t increment);

        /*! 
          Change value by multiplying the current value by the given factor. 
          Resulting value will be clipped to lower or upper bound accordingly.
          This will provide smoother blending on lower values
          @param factor in 1/100 : 100 = 1(no change) , 10= 0.10  1=0.01, 200=double, 300=triple
        */
        void multiply_value(int16_t factor);


        void trace_hsv();
        void trace_rgb(t_color_rgb_int color_rgb );

        /*!
          Calculate the current rgb values
          @return rgb values for the current color in as t_lamphsv_color_rgb 
        */
        t_lamphsv_color_rgb get_color_rgb(float maximum_value);

 protected:
        t_color_hsv m_color_hsv;
        
};
        
#endif
