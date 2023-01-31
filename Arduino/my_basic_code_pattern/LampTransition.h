#ifndef LAMPTRANSITION_H
#define LAMPTRANSITION_H

#include "mainSettings.h"


#ifdef TRACE_ON
//#define TRACE_LAMPTRANSITION_MODULATION
//#define TRACE_LAMPTRANSITION_EVENTS
#endif


#define LAMPTRANSITION_PENDING_INDICATOR 0xffff
#define LAMPTRANSITION_DONE_INDICATOR 0


enum transition_type_t {TT_NONE,TT_FADEIN,TT_FADEOUT,TT_BLEND};

/*! 
  class to manage and calculate a color transition for a rgb lamp. To save memory, time resolution for the 
  transition duration is reduced to 16ms steps (=62 frames per second) and can be max of 1048 seconds= 17 minutes
  Memory Footprint 48 Bytes per instance + 2 Bytes for timestamp static
*/
class LampTransition
{
  public:
    /*! 
      Constructor, setting all to black and no transition
    */
    LampTransition();

    /*!
      Set the current time value for all upcoming operations. This reduces the effort to provide the 
      current time in every method call and convert it to the internal scale. Setting the time externally enables
      better synchronization of all lamps. (messing this up can be achieved by calling setCurrentTime inbetween operations)
      @param current_time_ms : Current system time in ms
    */
    static setCurrentTime(uint32_t current_time_ms) {s_current_time_16ms=(int16_t)(current_time_ms>>4);};

    /*!
      set the current color. This will stop the the transition and set the color to the given values
      @param red : Amount of red in 8 bit (255= max value)
      @param green : Amount of green in 8 bit (255= max value)
      @param blue : Amount of green in 8 bit (255= max value)
      @
    */
    void setCurrentColor(uint8_t red, uint8_t green, uint8_t blue) {m_current_red=red; m_current_green=green; m_current_blue=blue; m_transition_duration_16ms=LAMPTRANSITION_DONE_INDICATOR;};

    /*!
      set the current color. This will stop the the transition and set the color to the given values.The transition
      duration will be set to 0.
      (more efficient pointer version, only pushes 2 bytes to the stack )
      @param pRGBColor : Pointer to a t_lamp_rgb_color struct
    */
    void setCurrentColor(t_lamp_rgb_color * pRGBColor) {m_current_red=pRGBColor->r; m_current_green=pRGBColor->g; m_current_blue=pRGBColor->b; 
                                                      m_transition_duration_16ms=LAMPTRANSITION_DONE_INDICATOR;
                                                      };

    /*!
      change the color to transition to. If already in transition, this will change the direction but not the timeline.
      At the end of the transition, the target color becomes the current color. If the target is equal to the current color, the lamp 
      will count as completly transitioned immediatly.
      @param red : Amount of red in 8 bit (255= max value)
      @param green : Amount of green in 8 bit (255= max value)
      @param blue : Amount of green in 8 bit (255= max value)
      @
    */
    void setTargetColor(uint8_t red, uint8_t green, uint8_t blue) { t_lamp_rgb_color myColor;myColor.r=red; myColor.g=green; myColor.b=blue;setTargetColor(&myColor);};

    /*!
      change the color to transition to. If already in transition, this will change the direction but not the timeline.
      At the end of the transition, the target color becomes the current color. If the target is equal to the current color, the lamp 
      will count as completly transitioned immediatly.
      (more efficient pointer version, only pushes 2 bytes to the stack )
      @param red : Amount of red in 8 bit (255= max value)
      @param green : Amount of green in 8 bit (255= max value)
      @param blue : Amount of green in 8 bit (255= max value)
      @
    */
    void setTargetColor(t_lamp_rgb_color * pRGBColor);


    /*!
      Initiate the transition from the current color to the target color for the given duration. last setCurrentTime will be used as start time
      @param duration_ms : Duration of the transition in ms (will be capped to 1048 seconds(17 Minutes) and reduced to 16ms granularity)
    */
    void startTransition(uint32_t duration_ms);

    /*!
      Immediatly jump to the final target color and end the transition.  The transition
      duration will be set to 0.
    */
    void finishTransition();

    /*! 
      Calculate and provide the transition color for the point in time set by setCurrentTime()
      @return rgb color type with the current values
    */
    t_lamp_rgb_color getModulatedColor() {t_lamp_rgb_color myColor; getModulatedColor(&myColor); return myColor;}; 

    /*! 
      Calculate and provide the transition color for the point in time set by setCurrentTime()
      @param pTarget Pointer ot a t_lamp_rgb_color variable type with the current values
    */
     void getModulatedColor(t_lamp_rgb_color *pTarget); 

    /*!
      Determine if the lamp is in transition. Can be used to prevent unnecessary calls to get current color
      @return True if lamp is in a transition process
    */
    bool isInTransition() {return m_transition_duration_16ms!=LAMPTRANSITION_DONE_INDICATOR && m_transition_duration_16ms!=LAMPTRANSITION_PENDING_INDICATOR;};

    /*!
      Determine if transition has not been started since the last setting of target colors.
      @return True if 
    */
    bool isTransitionPending() {return m_transition_duration_16ms==LAMPTRANSITION_PENDING_INDICATOR;};

  protected:
      uint16_t  m_start_transition_time_16ms=0;  // Point in time, the transition started  in internal scale
      uint16_t  m_transition_duration_16ms=0;  // Transition duration in internal scale, set to 0xffff for pending transition, set to 0 when transtion is complete
      uint8_t   m_current_red, m_current_green, m_current_blue;
      uint8_t   m_target_red, m_target_green, m_target_blue;

  private:
      static uint16_t s_current_time_16ms;  // The current time used in time specific method in internal scale

};



#endif /* End of Header file */
