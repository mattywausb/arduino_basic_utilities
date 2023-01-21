#include "mainSettings.h"
#include "Arduino.h"
#include "Encoder.h"


/* *** */
Encoder::Encoder() {
    byte m_prev_signal_state =0;   // state memory for the ISR
    m_change_amount = 0;    // Collection of changes by the ISR
    #ifdef ENCODER_H_TRACE
       byte m_signal_call_count=0;   // for debuggin, we can count ISR calls until the next trace output
    #endif 

    byte m_process_flags =ENCODER_H_WRAP|ENCODER_H_HIGH_IS_CLOSE_BIT|ENCODER_H_ENABLE_BIT;  // all configuration flags and the change flag are stored here
    configureRange(10,20,1,0);
    setValue(0);
 return;
}

/* ******* API ************* */

/* *** */
void  Encoder::configureCloseSignal(bool high_is_close)
{
  if(high_is_close) m_process_flags|=ENCODER_H_HIGH_IS_CLOSE_BIT;
  else m_process_flags&= ~ENCODER_H_HIGH_IS_CLOSE_BIT;
};


/* *** */
int16_t Encoder::configureRange(int16_t rangeMin, int16_t rangeMax, int16_t stepSize, byte wrap_mode)
{
  #ifdef ENCODER_H_TRACE
    Serial.print(F("TRACE_ENCODER configureRange("));
    Serial.print(rangeMin); Serial.print(F(","));
    Serial.print(rangeMax); Serial.print(F(") f="));
    Serial.println(0x0100|m_process_flags , BIN);
  #endif
  m_rangeMin = min(rangeMin, rangeMax);
  m_rangeMax = max(rangeMin, rangeMax);
  m_process_flags = (m_process_flags & ~ENCODER_H_WRAP_CLIPPED) | wrap_mode;
  m_stepSize = stepSize;
  #ifdef ENCODER_H_TRACE
    Serial.print(F("\tmin="));Serial.print(m_rangeMin); 
    Serial.print(F("\tmax="));Serial.print(m_rangeMax); 
    Serial.print(F("\tstep=")); Serial.print(m_stepSize);
    Serial.print(F("\tf'="));Serial.println(0x0100|m_process_flags , BIN);
  #endif
  setValue(m_value); //this will check the value and place it in the new range
}

/* *** */
int16_t Encoder::setValue(int16_t newValue) { // set the current value (whithin the defined bounds)
 m_value = newValue;
  if (m_value < m_rangeMin) m_value = m_rangeMin;
  if (m_value > m_rangeMax) m_value = m_rangeMax;
  m_process_flags &= ~ENCODER_H_PENDING_CHANGE_BIT;
  m_change_amount=0;

 #ifdef ENCODER_H_TRACE
    Serial.print(F("TRACE_ENCODER setValue("));
    Serial.print(newValue); Serial.print(F(") >"));
    Serial.println(m_value);
 #endif    

  return m_value;
}


/* *** */
int16_t Encoder::getValue() {
  m_process_flags &= ~ENCODER_H_PENDING_CHANGE_BIT;
  return m_value;
}; // calling get value will reset "pendingChangeFlag"

/* *********** Processing methods ************** */

 /* evaluate the signal and track the change counter  accordingly (This must be called by the ISR )*/
void Encoder::processSignal(byte clock_signal, byte direction_signal) //  evaluates the signal and tracks the change counter  accordingly
{
  if(!(m_process_flags&ENCODER_H_HIGH_IS_CLOSE_BIT)) {  // LOW is Closed, so invert incoming signal
    clock_signal= !clock_signal;
    direction_signal= !direction_signal;
  }

  #ifdef ENCODER_H_TRACE
    m_signal_call_count++;
  #endif
 
  #ifdef INPUT_FEEDBACK_ON_LED_BUILTIN
        digitalWrite(LED_BUILTIN, clock_signal);
  #endif

  if(((m_prev_signal_state&ENCODER_H_CLOCK_BIT)==0) && clock_signal) { //clock got closed
      if(direction_signal) m_prev_signal_state = ENCODER_H_DIRECTION_BIT|ENCODER_H_CLOCK_BIT;  
      else  m_prev_signal_state = ENCODER_H_CLOCK_BIT;
      return;
  }

  if((m_prev_signal_state&ENCODER_H_CLOCK_BIT) && !clock_signal) { //clock got opened 

      if(m_prev_signal_state&ENCODER_H_DIRECTION_BIT) { // direction was closed on clock close
        if (!direction_signal) { // and now it is open = encoder turned counter clockwise 
          m_change_amount-=1;
        }
      } else {  // direction was open on clock close
        if(direction_signal) { // and now it is closed =  turned  clockwise
          m_change_amount+=1;
        }
      }
      if(direction_signal) m_prev_signal_state = ENCODER_H_DIRECTION_BIT; //Clock open, 
      else  m_prev_signal_state = 0; // all open
  }


}


/* *** */
bool Encoder::processChange(){ 
  bool is_relevant_event=false;

  /* transfer high resolution encoder movement into tick encoder value */
  int8_t tick_encoder_change_value = m_change_amount;  // Freeze the value for upcoming operations
  #ifdef ENCODER_H_TRACE_CHANGE_HIGH
    Serial.print(F("TRACE_ENCODER_CHANGE_HIGH:"));
    Serial.print(F("v=")); Serial.print(tick_encoder_change_value);
    Serial.print(F("\tf=")); Serial.print(0x0100|m_process_flags,BIN);
    Serial.print(F("\ts=")); Serial.println(0x80|m_prev_signal_state,BIN);
  #endif

  if (tick_encoder_change_value) { // there are accumulated changes
    if((m_process_flags&ENCODER_H_ENABLE_BIT) && tick_encoder_change_value<ENCODER_MAX_CHANGE_PER_TICK && tick_encoder_change_value>-ENCODER_MAX_CHANGE_PER_TICK)   {
      m_value += tick_encoder_change_value * m_stepSize;
      // Wrap or limit the encoder value 
      while (m_value > m_rangeMax) m_value = (m_process_flags&ENCODER_H_WRAP_AT_MAX_BIT) ? (m_process_flags&ENCODER_H_CLIP_ON_MAX_WRAP_BIT) ? m_rangeMin: m_value-(m_rangeMax-m_rangeMin+1) : m_rangeMax;
      while (m_value < m_rangeMin) m_value = (m_process_flags&ENCODER_H_WRAP_AT_MIN_BIT) ? (m_process_flags&ENCODER_H_CLIP_ON_MIN_WRAP_BIT) ? m_rangeMax: m_value+(m_rangeMax-m_rangeMin+1) : m_rangeMin;

      m_process_flags |= ENCODER_H_PENDING_CHANGE_BIT;
      is_relevant_event=true;
    }
 
    m_change_amount -= tick_encoder_change_value; // remove the processed value from the tracking
    #ifdef ENCODER_H_TRACE
        Serial.print(F("TRACE_ENCODER processChange: f'="));Serial.print(0x0100|m_process_flags , BIN);
        Serial.print(F("\tm_signal_call_count=")); Serial.print(m_signal_call_count);
        Serial.print(F("\ttick_encoder_change_value=")); Serial.print(tick_encoder_change_value);
        Serial.print(F("\tm_value=")); Serial.print(m_value);
        Serial.print(F("\tm_change_amount left=")); Serial.println(m_change_amount);
        m_signal_call_count=0;
    #endif
  }

  return is_relevant_event;
}



