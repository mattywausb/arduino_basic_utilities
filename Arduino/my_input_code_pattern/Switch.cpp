 #include "Arduino.h"
 #include "Switch.h"

#define DEBOUNCE_INTERVAL 10
#define DURATION_CAP 32000
#define DURATION_CAP_UINT8 250
 
Switch::Switch() {
   m_state_flags=SWITCH_H_DURATION_CAP_BIT;
   m_millies_at_last_change=0; 
   m_last_duration=DURATION_CAP_UINT8;
 }

void Switch::configureCloseSignal(bool high_is_close)
{
  if(high_is_close) m_state_flags|=SWITCH_H_HIGH_IS_CLOSE_BIT;
  else m_state_flags&= ~SWITCH_H_HIGH_IS_CLOSE_BIT;
}

void Switch::processSignal(byte digital_readout) {

  bool switch_is_closed=(m_state_flags&SWITCH_H_HIGH_IS_CLOSE_BIT)?digital_readout:!digital_readout;
  uint16_t current_time=millis(); // we only care about the last 65536 milliseconds

  /* Manage state change */
  m_state_flags&=0xfd; // Remove change flag

  if(switch_is_closed) {
    if(!m_state_flags&SWITCH_H_STATE_BIT  ) { //was open in previous scan
        m_state_flags|=SWITCH_H_STATE_BIT  |SWITCH_H_CHANGE_BIT ; // Set change and state flag
      }
    } else  { // switch is open
    if(m_state_flags&SWITCH_H_STATE_BIT  ) { // was closed in previous scan
      if(current_time-m_millies_at_last_change<DEBOUNCE_INTERVAL) return; // Debounce first
      m_state_flags=(m_state_flags&~SWITCH_H_STATE_BIT  )|SWITCH_H_CHANGE_BIT ; // clear state flag and set change  flag
    }
   }
  
  /* Manage duration progress */
  uint16_t duration=(m_state_flags&SWITCH_H_DURATION_CAP_BIT)? DURATION_CAP : current_time-m_millies_at_last_change;
  if(duration>DURATION_CAP) { 
    duration=DURATION_CAP ;
      m_state_flags|=SWITCH_H_DURATION_CAP_BIT; // Set the duration cap bit
  }
}

/*
  if(m_state_flags&SWITCH_H_CHANGE_BIT)  { // Change noticed
    m_last_duration = duration >> 7; //Shift by 7 bit = divide by 128 = reduce resolution to 128ms
    m_millies_at_last_change = current_time;
    m_state_flags &= ~SWITCH_H_DURATION_CAP_BIT; // remove DURATION CAP flag
    }
    }
    } */

  





