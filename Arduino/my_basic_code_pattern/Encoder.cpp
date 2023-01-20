#include "mainSettings.h"
#include "Arduino.h"
#include "Encoder.h"

#ifdef TRACE_ON
  #define TRACE_ENCODER
#endif


/* *** */
Encoder::Encoder() {
 return;
}


/* *** */
void  Encoder::configureSignalmode(bool high_is_close)
{
  if(high_is_close) m_process_flags|=ENCODER_H_HIGH_IS_CLOSE_BIT;
  else m_process_flags&= ~ENCODER_H_HIGH_IS_CLOSE_BIT;
};


/* *** */
int16_t Encoder::configureRange(int16_t rangeMin, int16_t rangeMax, int16_t stepSize, byte wrap_mode)
{
  m_rangeMin = min(rangeMin, rangeMax);
  m_rangeMax = max(rangeMin, rangeMax);
  m_process_flags = (m_process_flags & ~ENCODER_H_WRAP_CLIPPED) | wrap_mode;
  m_stepSize = stepSize;
  setValue(m_value); //this will check the value and place it in the new range
  #ifdef TRACE_ENCODER
    Serial.print(F("TRACE_ENCODER configureRange:"));
    Serial.print(m_rangeMin); Serial.print(F("-"));
    Serial.print(m_rangeMax); Serial.print(F(" Step "));
    Serial.print(m_stepSize); Serial.print(F(" Wrap "));
    Serial.println(m_process_flags & ENCODER_H_WRAP_CLIPPED, BIN);
  #endif
}


 /* evaluate the signal and track the change counter  accordingly (This must be called by the ISR )*/
void Encoder::processSignal(byte clock_signal, byte direction_signal) //  evaluates the signal and tracks the change counter  accordingly
{
  if(!(m_process_flags&ENCODER_H_HIGH_IS_CLOSE_BIT)) {  // Flip signal to HIGH is closed logic
    clock_signal!=clock_signal;
    direction_signal!=direction_signal;
  }

  #ifdef TRACE_INPUT_ENCODER
    m_signal_call_count++;
  #endif
  #ifdef INPUT_FEEDBACK_ON_LED_BUILTIN
    digitalWrite(LED_BUILTIN, clock_signal);
  #endif

  if(!(m_prev_signal_state&ENCODER_H_CLOCK_BIT) && clock_signal) { //clock got closed
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
  if (tick_encoder_change_value) { // there are accumulated changes
    if(m_process_flags&ENCODER_H_ENABLE_BIT && tick_encoder_change_value<ENCODER_MAX_CHANGE_PER_TICK && tick_encoder_change_value>-ENCODER_MAX_CHANGE_PER_TICK)   {
      m_value += tick_encoder_change_value * m_stepSize;
      // Wrap or limit the encoder value 
      while (m_value > m_rangeMax) m_value = (m_process_flags&ENCODER_H_WRAP_AT_MAX_BIT) ? (m_process_flags&ENCODER_H_CLIP_ON_MAX_WRAP_BIT) ? m_rangeMin: m_value-(m_rangeMax-m_rangeMin) : m_rangeMax;
      while (m_value < m_rangeMin) m_value = (m_process_flags&ENCODER_H_WRAP_AT_MIN_BIT) ? (m_process_flags&ENCODER_H_CLIP_ON_MIN_WRAP_BIT) ? m_rangeMax: m_value+(m_rangeMax-m_rangeMin) : m_rangeMin;

      m_process_flags |= ENCODER_H_PENDING_CHANGE_BIT;
      is_relevant_event=true;
    }
    m_change_amount -= tick_encoder_change_value; // remove the processed value from the tracking
    #ifdef TRACE_INPUT_ENCODER
        Serial.print(F("TRACE_ENCODER processChange:"));
        Serial.print(F(" m_signal_call_count=")); Serial.print(m_signal_call_count);
        Serial.print(F("\ttick_encoder_change_value=")); Serial.print(tick_encoder_change_value);
        Serial.print(F("\tm_value=")); Serial.print(m_value);
        Serial.print(F("\tm_change_amount left=")); Serial.println(m_change_amount);
        m_signal_call_count=0;
    #endif
  }

  return is_relevant_event;
}


/* *** */
int16_t Encoder::setValue(int16_t newValue) { // set the current value (whithin the defined bounds)
 m_value = newValue;
  if (m_value < m_rangeMin) m_value = m_rangeMin;
  if (m_value > m_rangeMax) m_value = m_rangeMax;
  m_process_flags &= ~ENCODER_H_PENDING_CHANGE_BIT;
  m_change_amount=0;
  return m_value;
}


/* *** */
int16_t Encoder::getValue() {
  m_process_flags &= ~ENCODER_H_PENDING_CHANGE_BIT;
  return m_value;
}; // calling get value will reset "pendingChangeFlag"
