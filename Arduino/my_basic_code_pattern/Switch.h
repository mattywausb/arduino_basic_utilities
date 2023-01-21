#ifndef SWITCH_H
#define SWITCH_H



#ifdef TRACE_ON
   // #define TRACE_FLAG_CHANGE
#endif


#define SWITCH_H_CHANGE_BIT  0x02
#define SWITCH_H_STATE_BIT   0x01
#define SWITCH_H_CHANGE_CHECK_MASK 0x03
#define SWITCH_H_HIGH_RESOLUTION_DURATION_BIT 0x04
#define SWITCH_H_DURATION_CAP_BIT 0x08
#define SWITCH_H_HIGH_IS_CLOSE_BIT 0x80
#define SWITCH_H_COOLDOWN_BITS 0x70

/*!
*  Class to provide essential digital switch handling. HIGH/LOW Signal capture must be done externally and then passed to processSignal method
*  This way you have the full choice how to get the signal
*  Main features: debounce logic (with configurable cooldown time), duration tracking, change event isolation
*  Memory footprint: 4 Bytes per switch (5 bytes if flag change tracing is enabled)
*
*  Duration is stored in 8+1 bit with a resolution of 16ms for up to 4080ms and 128ms for up to  32000ms 
*  Reading functions scale the duration value to ms, so it is compatible to the common time handling
*/
class Switch 
{
  public:
   /*! 
      Constructor
    */
    Switch();


    /*!
      Define what signal (HIGH/LOW) will represent the closed state of the switch. 
      Depends if the pin is connected in a "PULLDOWN"(HIGH=CLOSE) or PULLUP (DOWN=CLOSE) circuit to the switch.
      Setting this wrong will induce inverted results and disable the debounce mechanic
      @param bool high_is_close bool: if true, the HIGH signal is equal to a closed contact
    */
    void configureCloseSignal(bool high_is_close);

    /*!
      Defines the time to wait after switch close until an open is counted as real. The value can be from 0 to 112 and
      will be stored in a granularity of 16ms.
      @param DebounceWaittime positive 8 Bit integer. Will be masked to 0x70. Negative values will be changed to 0
      @return the final value stored
    */
    int8_t configureDebounceWaittime(int8_t DebounceWaittime);

    /*!
      evaluate the signal and track the changes accordingly (This must be called regulary by the loop)
      @param digital_readout HIGH/LOW signal from the pin 
      @return true if there was a change detected
    */
    bool processSignal(byte digital_readout); 

    /*!
      Determine switch state
      @return true when the switch is currently in a close state
    */
    bool isClosed() { return m_state_flags&SWITCH_H_STATE_BIT;};

    /*!
      Determine closing switch change event
      @return true when the switch changed from open to closed in the previous processing call
    */
    bool gotClosed() { return (m_state_flags&SWITCH_H_CHANGE_CHECK_MASK)==SWITCH_H_CHANGE_CHECK_MASK;}; 

    /*!
      Determine switch state
      @return true when the switch is currently in a open state (not closed)
    */
    bool isOpen() { return ! m_state_flags&SWITCH_H_STATE_BIT;};

    /*!
      Determine opening switch change event
      @return true when the switch changed from closed to open in the previous processing call
    */
    bool gotOpened() { return (m_state_flags&SWITCH_H_CHANGE_CHECK_MASK)==SWITCH_H_CHANGE_BIT;};

    /*!
      Determine any switch change event
      @return true when the switch changed in any direction in the previous processing call
    */
    bool gotChanged() { return m_state_flags&SWITCH_H_CHANGE_BIT;};

    /*!
      Determine duration of the current or previous close state
      @return // duration of the current or last close state in ms
    */
    uint16_t getClosedDuration(); 

    /*!
      Determine duration of the current or previous open state
      @return // duration of the current or last open state in ms
    */
    uint16_t getOpenDuration(); // duration of the current or last open phase


  private:
    uint8_t m_last_duration;  // Last duration in 128ms 
    byte  m_state_flags;   // ---h-tsc   h=high_is_close, t=duration is capped at max of 30s, s=switch changed state to previous, c=current state
    uint16_t m_millies_at_last_change;  // last 16 bit of timestamp, when state changed
    #ifdef TRACE_FLAG_CHANGE
      uint8_t m_trace_prev_flags;
    #endif
};

#endif