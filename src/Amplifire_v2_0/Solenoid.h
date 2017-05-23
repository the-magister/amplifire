#ifndef Solenoid_h
#define Solenoid_h

#include <Arduino.h>
#include <Streaming.h>
#include <Metro.h>

#define MIN_DURATION 10UL
#define MAX_DURATION 2000UL
#define MAX_CYCLES 10UL

class Solenoid {
  public: 
    // setup
    void begin(byte pin, byte offDefinition); // set the pin
    void set(unsigned long onDuration, unsigned long offDuration, byte cycles);

    // information
    void show(); // show timings
    
    // operations
    void arm(); // allow firing of the solenoid
    void disarm(); // disallow firing of the solenoid.
    
    void start(); // start the run
    boolean running(); // returns false if we're done
    boolean isFiring(); // returns true if the firing pin is on.
    void stop(); // halt any time

    void purge();  // you should never use this.

  private:
    byte pin, off, on, cyclesTotal,  remainingCycles;

    unsigned long onDuration, offDuration;

    Metro timer;

    boolean isOn, isArmed;

    void setOff();
    void setOn();
    void toggle();
    
};

#endif
