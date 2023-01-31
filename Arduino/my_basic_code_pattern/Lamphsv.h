#ifndef LAMPHSV_H
#define LAMPHSV_H
#include "Arduino.h"
#include "mainSettings.h"


#ifdef TRACE_ON
    #define LAMPHSV_ADD_TRACE_METHODS
    //#define LAMPHSV_TRACE_SET_CALIBRATION
    //#define LAMPHSV_TRACE_CALIBRATION
#endif

#define LAMPHSV_HUE_RED 0
#define LAMPHSV_HUE_ORANGE 15.0
#define LAMPHSV_HUE_YELLOW 45.0
#define LAMPHSV_HUE_GREEN 120.0
#define LAMPHSV_HUE_CYAN 180.0
#define LAMPHSV_HUE_SKYBLUE 220.0
#define LAMPHSV_HUE_BLUE 240.0
#define LAMPHSV_HUE_PURPLE 250.0
#define LAMPHSV_HUE_MAGENTA 300.0
#define LAMPHSV_HUE_PINK 340.0

#define LAMPHSV_CALIBRATION_INDEX_YELLOW 0
#define LAMPHSV_CALIBRATION_INDEX_CYAN 1
#define LAMPHSV_CALIBRATION_INDEX_MAGENTA 2

#define LAMPHSV_CHANGE_BIT 0x4000
#define LAMPHSV_HUE_SCALE 6144
#define LAMPHSV_HUE_DOWNSCALE_CORRECTION_BORDER 4334
#define LAMPHSV_SATURATION_INTERNAL_MAX 128
#define LAMPHSV_VALUE_INTERNAL_MAX 128
#define LAMPHSV_8BIT_DECIMALS 7

#ifndef T_LAMP_RGB_COLOR
#define T_LAMP_RGB_COLOR

#endif



typedef struct {
    int16_t h;       // 0-360 Degree  (internally scaled to 6144, wichs makes 1024 to be 60 degrees for fast integer arithmetic)
    uint8_t s;  // 0-100 with 100 equivalent to 1.00 (internally scaled to 0-128 for fast integer arithmetic)
    uint8_t v;       // 0-100 with 100 equivalent to 1.00 (internally scaled to 0-128 for fast integer arithmetic)
} t_lamp_hsv_color;

/*!
  Store and manipulate color of a lamp as hue(in degree), saturation and value(=intensity). Provide RGB value for sending to 
  displays. Implementation with fast integer arithmetic and low memory footprint. (4 Bytes per lamp)
*/

class Lamphsv {
 public:
        /*!
          Constructor
        */
        Lamphsv(void);

        /*! 
          Set hsv values of the lamp
          @param hue Define the hue in degrees (from 0 to 359)
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
        void set_hsv(float hue,float saturation,float value) {set_hsv((int16_t)(hue),(uint8_t)(saturation*100.0),(uint8_t)(value*100.0));};

        /*! 
          Get hue value 
          @return hue value in 1/10 degress (0-3599)
        */
        int16_t get_hue(); 




        /*! 
          Set hue to  a specific angle. Values out of bound will be set to 0.
          @param hue Define the hue in degrees (from 0 to 359)
        */
        void set_hue(int16_t angle) ;
        
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
        int8_t get_saturation() ;

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
        int8_t get_value()  ;

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

        /*!
          Check, if color has changed since the last get_color_rgb
          @return rgb values for the current color in as t_lamphsv_color_rgb 
        */

        bool is_changed() {return ((m_hue_60_d10 & LAMPHSV_CHANGE_BIT)!=0);}

        /*!
          Get the current rgb values. This will reset the is_changed flag
          @return rgb values for the current color in as t_lamphsv_color_rgb 
        */
        t_lamp_rgb_color get_color_rgb() ;

        /*!
          Provide the current rgb values in a given buffer of t_lamphsv_color_rgb. This will reset the is_changed flag.
          Since the result normally is instantly pushed to a neopixel or other function, using the pointer will take less time for value transfer in memory,
          @param pTarget pointer to a t_lamp_rgb_color variable for storing the rgb values
        */
        void get_color_rgb(t_lamp_rgb_color *pTarget);

        #ifdef LAMPHSV_ADD_TRACE_METHODS
        /*!
          Provide a complete diagnosis on serial
        */
          void print_members_to_serial();
          //void print_rgb_to_serial();
        #endif

        /*! 
          Set the uncalibrated hue, the led is really showing yellow, cyan and magenta. This will
          be used to corrext the conversion, to provide yellow,cyan, magenta on the hue 60,180,300. This will compensate 
          for different brightness of the colors. Setting these value might not be necessary, when the initial values in Lamphsv.cpp 
          match the behaviour of the used LED's
          @param angle_index The index of the color, this angle will be used 0=yellow, 1=cyan, 2 = magenta
          @param angle The real(uncompesated) angle the color is shown
        */
       static void setCalibrationHueAngle(uint8_t angle_index, uint16_t angle);

       /*!
        Use the defined hue 
       */
       static void enableCalibration() {s_use_calibration=true;};
       static void disableCalibration() {s_use_calibration=false;};
       static boolean getCalibrationState() {return s_use_calibration;};


 protected:
        int16_t m_hue_60_d10;  // hue scaled so it has 60 degrees at bit 11 (10 decimals)
        uint8_t m_saturation_d7;
        uint8_t m_value_d7;

  private:
        /*! 
          Get current hue value calibrated
          @return hue angle
        */
        int16_t get_calibrated_hue(); 

       static uint16_t s_calibration_angle_60_d10[3]; // max values at 60,180,300 Degree for red,green, green,blue, blue,red transition
       static bool s_use_calibration;

};
        
#endif
