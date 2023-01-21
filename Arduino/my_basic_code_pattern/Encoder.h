#ifndef ENCODER_H
#define ENCODER_H

/*!
*  Class to provide essential encoder  handling. Capture of CLOCK and DIRECTION signal  must be done externally via ISR and then passed to processSignal method
*  Main features: Manage value to be in e specifig range. Step size of the value. Multiple wrap around configurations
*  Memory footprint: 10 bytes
*/

#ifdef TRACE_ON
   #define ENCODER_H_TRACE
 //  #define ENCODER_H_TRACE_CHANGE_HIGH
#endif


#define ENCODER_H_CLOCK_BIT  0x01
#define ENCODER_H_DIRECTION_BIT   0x02

#define ENCODER_H_HIGH_IS_CLOSE_BIT 0x80
#define ENCODER_H_ENABLE_BIT 0x40
#define ENCODER_H_PENDING_CHANGE_BIT 0x01
#define ENCODER_H_WRAP_AT_MAX_BIT 0x20
#define ENCODER_H_WRAP_AT_MIN_BIT 0x10
#define ENCODER_H_CLIP_ON_MAX_WRAP_BIT 0x08
#define ENCODER_H_CLIP_ON_MIN_WRAP_BIT 0x04

#define ENCODER_H_NO_WRAP 0x00
#define ENCODER_H_WRAP 0x30
#define ENCODER_H_WRAP_CLIPPED 0x3c

#define ENCODER_MAX_CHANGE_PER_TICK 100

class Encoder 
{
  public:
    /*! 
      Constructor
    */
    Encoder();

    /*!
      Define what signal (HIGH/LOW) will represent the closed state of the switch. This depends of state is detected in PULLDOWN oder PULLUP logic)
      Setting this wrong will result in an "undetected step" at start and every time you change direction
      @param bool high_is_close bool: if true, the HIGH signal is equal to a closed contact
    */
    void configureCloseSignal(bool high_is_close);

    /*!
      Set range of values, stepsize and wrap behaviour
      @param rangeMin int16_t: Defines the lowest value 
      @param rangeMax int16_t: Defines the highst value 
      @param stepSize int16_t: Value change for every encoder step 
      @param wrap_mode byte: Bitflags defining the wrapping behaviour, when the value reaches the border
                        If bit is set, when reaching the border the value will wrap to the opposite border
                        ENCODER_H_WRAP_AT_MAX_BIT, ENCODER_H_WRAP_AT_MIN_BIT, ENCODER_H_WRAP = MIN and MAX Wrap
                        If Clipping is set, the Wrap will start exactliy with the border value, regardles of more steps
                        ENCODER_H_CLIP_ON_MAX_WRAP_BIT,ENCODER_H_CLIP_ON_MIN_WRAP_BIT, ENCODER_H_WRAP_CLIPPED
      @return int16_t: the current value, which might have been adjustet to be in the defined borders
    */
    int16_t configureRange(int16_t rangeMin, int16_t rangeMax, int16_t stepSize, byte wrap_mode); 

    /*!
      Process the currently received signals. Must be cally by an ISR, that is bound on the CHANGE of the clock signal
      @param clock_readout byte: Signal of the clock pin of the encoder
      @param direction_readout byte: Signal of the direction pin of the encoder
    */
    void processSignal(byte clock_readout, byte direction_readout); 

    /*!
      Process all changes collected by processSignal and update the internal state accordingly. Must be called regulary by loop 
      @return bool: true if there was a value change. (useful for overall tracking of user input change times)
    */
    bool processChange(); // this evaluates the signal and updates states accordingly

    /*!
      Enable the processing of changes
    */
    void enable() { m_process_flags |= ENCODER_H_ENABLE_BIT;};   

    /*!
      Disable the processing of changes. This will discard further incoming signals until enable is called again.
    */
    void disable(){ m_process_flags &= ~ENCODER_H_ENABLE_BIT;}; 

    /*!
      Set the current value of the encoder. This will discard any pending changes. The value will be corrected to 
      the configured bounds if out of bound.
      @param newValue int: the new value
      @return int the final value stored
    */
    int16_t setValue(int16_t newValue); // set the current value (whithin the defined bounds)

    /*!
      Get the current value. This will also reset the change flag
      @return int: the current encoder value
    */

    int16_t getValue(); // calling get value will reset "pendingChangeFlag"
    
    /*!
      Determine if the value has changed since the last "getValue" call
      @return bool: true if the encoder value has been changed since the last getValue call
  
    */
    bool hasPendingChange() { return m_process_flags&ENCODER_H_PENDING_CHANGE_BIT;};

  private:
    volatile byte m_prev_signal_state =0;   // state memory for the ISR
    volatile int8_t m_change_amount = 0;    // Collection of changes by the ISR
    #ifdef ENCODER_H_TRACE
      volatile byte m_signal_call_count=0;   // for debuggin, we can count ISR calls until the next trace output
    #endif 

    byte m_process_flags =ENCODER_H_WRAP|ENCODER_H_HIGH_IS_CLOSE_BIT|ENCODER_H_ENABLE_BIT;  // all configuration flags and the change flag are stored here
    int16_t m_value = 0;      // the current value of the encoder
    int16_t m_rangeMin = 0;   // first valid value
    int16_t m_rangeMax = 99;  // last valid value
    int16_t m_stepSize = 1;   // step factor, used during translation of change_amount to value
};

#endif