#include "mainSettings.h"
#include "Arduino.h"
#include "Switch.h"

#ifdef TRACE_ON
    //#define TRACE_SWITCH_CHANGE
#endif


#define DURATION_CAP 32000
#define HI_RESOLUTION_LIMIT 4080
#define DURATION_CAP_UINT8 250
#define DEFAULT_DEBOUNCE_COOLDOWN 0x40
 
Switch::Switch() {
   m_state_flags=SWITCH_H_DURATION_CAP_BIT|DEFAULT_DEBOUNCE_COOLDOWN;
   #ifdef TRACE_FLAG_CHANGE
   m_trace_prev_flags=m_state_flags;
   #endif
   m_millies_at_last_change=0; 
   m_last_duration=DURATION_CAP_UINT8;
 }


/* configureCloseSignal */
void Switch::configureCloseSignal(bool high_is_close)
{
  if(high_is_close) m_state_flags|=SWITCH_H_HIGH_IS_CLOSE_BIT;
  else m_state_flags&= ~SWITCH_H_HIGH_IS_CLOSE_BIT;
}

/* configureDebounceWaittime */
int8_t Switch::configureDebounceWaittime(int DebounceWaittime)
{
  if(DebounceWaittime<0) DebounceWaittime=0;
  if(DebounceWaittime>127) DebounceWaittime=127;
  m_state_flags&=~ SWITCH_H_COOLDOWN_BITS ; // Remove previous setting
  m_state_flags|= (DebounceWaittime&SWITCH_H_COOLDOWN_BITS); // Set new value (but only in the resolution allowed)
  return (m_state_flags&SWITCH_H_COOLDOWN_BITS);
}

bool Switch::processSignal(byte digital_readout) {

  bool switch_is_closed=(m_state_flags&SWITCH_H_HIGH_IS_CLOSE_BIT)?digital_readout:!digital_readout;
  uint16_t current_time=millis(); // we only care about the last 65536 milliseconds

  /* Manage state change */
  m_state_flags&=0xfd; // Remove change flag

  if(switch_is_closed) {
    if(!(m_state_flags&SWITCH_H_STATE_BIT)  ) { //was open in previous scan
      #ifdef TRACE_SWITCH_CHANGE
        Serial.println(F("TRACE_SWITCH_CHANGE> switch got closed"));
      #endif
        m_state_flags|=SWITCH_H_STATE_BIT  |SWITCH_H_CHANGE_BIT ; // Set change and state flag
      }
    } else  { // switch is open
    if(m_state_flags&SWITCH_H_STATE_BIT  ) { // was closed in previous scan
      if(current_time-m_millies_at_last_change<(m_state_flags&SWITCH_H_COOLDOWN_BITS)) return; // Debounce first
      #ifdef TRACE_SWITCH_CHANGE
        Serial.println(F("TRACE_SWITCH_CHANGE> switch got opened after debounce"));
      #endif
      m_state_flags=(m_state_flags&~SWITCH_H_STATE_BIT  )|SWITCH_H_CHANGE_BIT ; // clear state flag and set change  flag
    }
   }
  
  /* Manage duration progress */
  uint16_t duration=(m_state_flags&SWITCH_H_DURATION_CAP_BIT)? DURATION_CAP : current_time-m_millies_at_last_change;
  if(duration>DURATION_CAP) { 
    duration=DURATION_CAP ;
      m_state_flags|=SWITCH_H_DURATION_CAP_BIT; // Set the duration cap bit
      #ifdef TRACE_SWITCH_DURATION
        Serial.println(F("TRACE_SWITCH_DURATION> switch reached DURATION_CAP"));
      #endif
  }

  /* manage duration memory */
  if(m_state_flags&SWITCH_H_CHANGE_BIT)  { // Change noticed
    if(duration<=HI_RESOLUTION_LIMIT) {
        m_last_duration = duration >> 4 ;// high precision tracking Shift by 4 bit = divide by 16
        m_state_flags |=  SWITCH_H_HIGH_RESOLUTION_DURATION_BIT;
    }  else  {
      m_last_duration = duration >> 7; // low precisions tracking Shift by 7 bit = divide by 128 = reduce resolution to 128ms
      m_state_flags &=  ~SWITCH_H_HIGH_RESOLUTION_DURATION_BIT;
    }
    m_millies_at_last_change = current_time;
    m_state_flags &= ~SWITCH_H_DURATION_CAP_BIT; // remove DURATION CAP flag
    }
  #ifdef TRACE_FLAG_CHANGE
      if(m_trace_prev_flags^m_state_flags) {
        Serial.print(F("TRACE_FLAG_CHANGE>new flags:  "));Serial.println(0x80|m_state_flags,BIN);
        m_trace_prev_flags=m_state_flags;
      }
  #endif
  return (m_state_flags&SWITCH_H_CHANGE_BIT);
}


uint16_t Switch::getClosedDuration() {
  uint16_t duration;
  if(m_state_flags&SWITCH_H_STATE_BIT) { // is currently closed, so we use current time - last change
      uint16_t current_time=millis();
      duration=(m_state_flags&SWITCH_H_DURATION_CAP_BIT)? DURATION_CAP : current_time-m_millies_at_last_change;
      if(duration>DURATION_CAP) duration=DURATION_CAP ;
  } else { // is open, so we use stored duration value
      duration=m_last_duration;
      if(m_state_flags&SWITCH_H_HIGH_RESOLUTION_DURATION_BIT)  duration<<=4;
      else duration<<=7;
  }
  return duration;
};



uint16_t Switch::getOpenDuration() {
  uint16_t duration;
  if(!(m_state_flags&SWITCH_H_STATE_BIT)) { // is currently open, so we use current time - last change
      uint16_t current_time=millis();
      duration=(m_state_flags&SWITCH_H_DURATION_CAP_BIT)? DURATION_CAP : current_time-m_millies_at_last_change;
      if(duration>DURATION_CAP) duration=DURATION_CAP ;
  } else { // is closed, so we use stored duration value from the previous open phase
      duration=m_last_duration;
      if(m_state_flags&SWITCH_H_HIGH_RESOLUTION_DURATION_BIT)  duration<<=4;
      else duration<<=7;
  }
  return duration;
}
    

  





