 #include "Arduino.h"
 #include "Button.h"

#define TIME_CAPPED_VALUE 0xfffe
#define HIGH_IS_CLOSE_FLAG 0x0004
#define DEBOUNCE_INTERVAL 10
 
Button::Button(bool high_is_close) {
   if(high_is_close) m_duration_and_flags=HIGH_IS_CLOSE_FLAG;
   else  m_duration_and_flags=0;
   m_millies_on_last_event=TIME_CAPPED_VALUE; // this value declares 
 }

void Button::processSignal(byte digital_readout) {

  bool button_is_closed=(m_duration_and_flags&HIGH_IS_CLOSE_FLAG)?digital_readout:!digital_readout;
  uint16_t current_time=millis(); // we only care about the last 65536 milliseconds

  m_duration_and_flags&=0xfffd; // Remove change flag

  if(button_is_closed) {
    if(!m_duration_and_flags&0x0001) { //was open in previous scan
        m_duration_and_flags|=0x003; // Set change and state flag
    }
  } else  { // button is open
    if(m_duration_and_flags&0x0002) { // was closed in previous scan
      if(current_time>m_millies_on_last_event+DEBOUNCE_INTERVAL) return; // Debounce first
      m_duration_and_flags=m_duration_and_flags&0xfffe|0x0002; // clear state flag and set change  flag
    }
  }
  
  // todo code 30s cap 
  
  uint16_t duration=current_time-m_millies_on_last_event;
  if(m_duration_and_flags&0x0002) { // Change noticed
    // Manage timing
    
    m_duration_and_flags=(duration&0xfff8)|(m_duration_and_flags&0x0007)
  }
  }
  




}
