#ifndef BUTTON_h
#define BUTTON_h

/*
*  Class to provide essential digital button handling. HIGH/LOW Signal capture must be done externally, so you have the full choice how to get the signal
*  Main features: debounce logic, duration tracking, event isolation
*
*  To save memory, the time resolution and boundaries are as follows
*  duration is stored in 8ms resolution with 13 bit and capped at 40s (6000). Function scales the value to ms to stay in the general pattern
*/

class Button 
{
  public:
  Button(bool /*high_is_close*/);
  void processSignal(byte /*digital_readout*/); // this evaluates the signal and updates states accordingly
  bool isClosed() { return m_duration_and_flags&0x0001;};
  bool gotClosed() { return (m_duration_and_flags&0x0003)==0x0003;}; // true, when button changed to closed state on last scan
  bool isOpen() { return ! m_duration_and_flags&0x0001;};
  bool gotOpened() { return (m_duration_and_flags&0x0003)==0x0002;}; // true, when button changed to open state on last scan
  uint16_t getCloseDuration(); // duration of the current or last close phase
  uint16_t getOpenDuration(); // duration of the current or last open phase

  private:
  uint16_t m_duration_and_flags;  // Bits of this variable a used as follows: dddd dddd dddd dhsc  (d=duration, h=high is close flag, s=state switch flag, c=current state)
  uint16_t m_millies_on_last_event;
};

#endif