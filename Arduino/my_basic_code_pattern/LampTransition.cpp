#include "Arduino.h"
#include "LampTransition.h"
#include "mainSettings.h"


uint16_t LampTransition::s_current_time_16ms=0; 

LampTransition::LampTransition()
{
  m_current_red=
  m_current_green=
  m_current_blue=
  m_target_red=
  m_target_green=
  m_target_blue=0;
  m_transition_duration_16ms=0;
}


void  LampTransition::getCurrentColor(t_lamp_rgb_color *pTarget)
{
   
   if(!isInTransition()) {  // return current color when not in transition
     pTarget->r=m_current_red;
     pTarget->g=m_current_green;
     pTarget->b=m_current_blue;
     return;
   }

   uint16_t current_transition_duration_16ms=s_current_time_16ms-m_start_transition_time_16ms;
   
   if(current_transition_duration_16ms>=m_transition_duration_16ms) { // end transition when duration is over
     finishTransition();
     pTarget->r=m_current_red;
     pTarget->g=m_current_green;
     pTarget->b=m_current_blue;
     return;
   }
   int16_t timeline_position_256=(((uint32_t)current_transition_duration_16ms)<<8)/m_transition_duration_16ms; // Determine relative position in timeline on a 256 scale (8bit)

   pTarget->r=(int16_t)m_current_red   - (((int16_t)m_current_red-  (int16_t)m_target_red)   * timeline_position_256)>>8;
   pTarget->g=(int16_t)m_current_green - (((int16_t)m_current_green-(int16_t)m_target_green) * timeline_position_256)>>8;
   pTarget->b=(int16_t)m_current_blue  - (((int16_t)m_current_blue- (int16_t)m_target_blue)  * timeline_position_256)>>8;
  
}



void LampTransition::startTransition(uint32_t duration_ms)
{
  #ifdef TRACE_PICTURELAMP_OPERATIONS
    Serial.print(F("TRACE_PICTURELAMP_OPERATIONS::startTransition for "));
    Serial.println(duration);
  #endif
  if(duration_ms>=LAMPTRANSITION_PENDING_INDICATOR) duration_ms=LAMPTRANSITION_PENDING_INDICATOR-1; // cap duration
  m_transition_duration_16ms=(int16_t)(duration_ms>>4);
  m_start_transition_time_16ms=s_current_time_16ms;
}

void LampTransition::finishTransition() 
{
  #ifdef TRACE_PICTURELAMP_OPERATIONS
    Serial.println(F("TRACE_PICTURELAMP_OPERATIONS::finishTransition"));
  #endif
    m_current_red=m_target_red;
    m_current_green=m_target_green;
    m_current_blue=m_target_blue;
    m_transition_duration_16ms=LAMPTRANSITION_DONE_INDICATOR;
};



