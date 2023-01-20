#ifndef ENCODER_H
#define ENCODER_H

/*
*  Class to provide essential encoder  handling. Capture of CLOCK and DIRECTION signal  must be done externally via ISR and then passed to processSignal method
*  Main features: Manage value to be in e specifig range. Step size of the value. Multiple wrap around configurations
*  Memory footprint: 
*/

#ifdef TRACE_ON
   // #define TRACE_FLAG_CHANGE
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

#define ENCODER_H_WRAP 0x30
#define ENCODER_H_WRAP_CLIPPED 0x3c

#define ENCODER_MAX_CHANGE_PER_TICK 100

class Encoder 
{
  public:
    Encoder();
    void configureCloseSignal(bool/*high_is_close*/);
    int configureRange(int /*rangeMin*/, int /*rangeMax*/, int /*stepSize*/, byte /*wrap_mode*/); // Set the range bounds, stepping and wrap behaviour
    void processSignal(byte /*clock_readout*/, byte /*direction_readout*/); // evaluate the signal and tracks the change counter  accordingly
    void processChange(); // this evaluates the signal and updates states accordingly
    void enable() { m_process_flags|=ENCODER_H_ENABLE_BIT;};   // allows tracking of changes
    void disable(){ m_process_flags&= ~ENCODER_H_ENABLE_BIT;};  // ignores all signals
    int setValue(int /*newValue*/); // set the current value (whithin the defined bounds)
    int getValue(); // calling get value will reset "pendingChangeFlag"
    bool hasPendingChange() { return m_process_flags&ENCODER_H_PENDING_CHANGE_BIT;};

  private:
    volatile byte m_prev_signal_state =0;
    volatile int8_t m_change_amount = 0;
    #ifdef TRACE_INPUT_ENCODER
      volatile byte m_signal_call_count=0;
    #endif 

    byte m_process_flags =ENCODER_H_WRAP|ENCODER_H_HIGH_IS_CLOSE_BIT|ENCODER_H_ENABLE_BIT;
    int m_value = 0;
    int m_rangeMin = 0;
    int m_rangeMax = 99;
    int m_stepSize = 1;
};

#endif