#ifndef BUTTON_h
#define BUTTON_h

/*
*  Class to provide essential digital button handling. HIGH/LOW Signal capture must be done externally, so you have the full choice how to get the signal
*  Main features: debounce logic, duration tracking, event isolation
*
*  To save memory, the time resolution and boundaries are as follows
*  duration is stored in 8 bit with a resoluition of 128ms (that would be more then enough for user interfaces) what results in a range of 0 to 32.783ms 
*  Function scales the value to ms to stay in the general pattern
*/

class Button 
{
  public:
  Button(bool /*high_is_close*/);
  void processSignal(byte /*digital_readout*/); // this evaluates the signal and updates states accordingly
  bool isClosed() { return m_state_flags&0x01;};
  bool gotClosed() { return (m_state_flags&0x03)==0x03;}; // true, when button changed to closed state on last scan
  bool isOpen() { return ! m_state_flags&0x01;};
  bool gotOpened() { return (m_state_flags&0x03)==0x02;}; // true, when button changed to open state on last scan
  uint16_t getCloseDuration(); // duration of the current or last close phase
  uint16_t getOpenDuration(); // duration of the current or last open phase

  private:
  uint8_t m_last_duration;
  byte  m_state_flags;   // ---h-tsc   h=high_is_close, t=duration is capped at max of 30s, s=switch changed state to previous, c=current state
  uint16_t m_millies_at_last_change;  // last 16 bit of timestamp, when state changed
};

#endif