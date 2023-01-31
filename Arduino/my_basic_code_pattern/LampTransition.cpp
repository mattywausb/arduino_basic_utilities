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


void LampTransition::setTargetColor(t_lamp_rgb_color * pRGBColor)
{   m_target_red= pRGBColor->r;
    m_target_green=pRGBColor->g;
    m_target_blue=pRGBColor->b; 
    if( m_current_red==m_target_red && m_current_green==m_target_green && m_current_blue==m_target_blue) {
      m_transition_duration_16ms=LAMPTRANSITION_DONE_INDICATOR;
      return;
    }
   if(!isInTransition()) m_transition_duration_16ms=LAMPTRANSITION_PENDING_INDICATOR;
};


void  LampTransition::getModulatedColor(t_lamp_rgb_color *pTarget)
{
   
    #ifdef TRACE_LAMPTRANSITION_MODULATION
        Serial.print(F("("));Serial.print(m_current_red);Serial.print(F("->"));Serial.print(m_target_red);
        Serial.print(F(","));Serial.print(m_current_green);Serial.print(F("->"));Serial.print(m_target_green);
        Serial.print(F(","));Serial.print(m_current_blue);Serial.print(F("->"));Serial.print(m_target_blue);
        Serial.print(F(")"));
    #endif

   if(!isInTransition() || s_current_time_16ms==m_start_transition_time_16ms) {  // return current color when not in transition or at exact start
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

   #ifdef TRACE_LAMPTRANSITION_MODULATION
      Serial.print(F(" t_256="));Serial.print(timeline_position_256);
      Serial.print(F(" dr="));Serial.print((int16_t)m_target_red   - (int16_t)m_current_red   );
      Serial.print(F(" dg="));Serial.print((int16_t)m_target_green   - (int16_t)m_current_green  );
      Serial.print(F(" db="));Serial.print((int16_t)m_target_blue   - (int16_t)m_current_blue  );
   #endif
             //        1---------------------------2-3-4----------------difference------------------------4---weighted--------------3-/256-21           
   pTarget->r=(uint8_t)((int16_t)m_current_red   + ( ( ((int16_t)m_target_red   - (int16_t)m_current_red  ) * timeline_position_256 ) >>8  ));
   pTarget->g=(uint8_t)((int16_t)m_current_green + ( ( ((int16_t)m_target_green - (int16_t)m_current_green) * timeline_position_256 ) >>8  ));
   pTarget->b=(uint8_t)((int16_t)m_current_blue  + ( ( ((int16_t)m_target_blue  - (int16_t)m_current_blue ) * timeline_position_256 ) >>8  ));
   
}



void LampTransition::startTransition(uint32_t duration_ms)
{
  #ifdef TRACE_LAMPTRANSITION_EVENTS
    Serial.print(F("TRACE_LAMPTRANSITION_EVENTS::startTransition for "));
    Serial.println(duration_ms);
  #endif
  if(m_current_red==m_target_red && m_current_green==m_target_green && m_current_blue==m_target_blue) // nothing to transition here
  {
    m_transition_duration_16ms=LAMPTRANSITION_DONE_INDICATOR;
      #ifdef TRACE_LAMPTRANSITION_EVENTS
        Serial.println(F("TRACE_LAMPTRANSITION_EVENTS::Target already reached, no transition needed "));
      #endif
    return;
  }
  if(duration_ms==0) {
    finishTransition();
    return;
  }
  if(duration_ms>=LAMPTRANSITION_PENDING_INDICATOR) duration_ms=LAMPTRANSITION_PENDING_INDICATOR-1; // cap duration
  m_transition_duration_16ms=(int16_t)(duration_ms>>4);
  m_start_transition_time_16ms=s_current_time_16ms;
}

void LampTransition::finishTransition() 
{
  #ifdef TRACE_LAMPTRANSITION_EVENTS
    Serial.println(F("TRACE_PICTURELAMP_OPERATIONS::finishTransition"));
  #endif
    m_current_red=m_target_red;
    m_current_green=m_target_green;
    m_current_blue=m_target_blue;
    m_transition_duration_16ms=LAMPTRANSITION_DONE_INDICATOR;
};



