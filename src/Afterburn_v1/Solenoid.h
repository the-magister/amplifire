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
    void begin(byte pin, byte offDefinition=LOW); // set the pin
    void set(unsigned long onDuration, unsigned long offDuration, byte cycles=1, unsigned long startDelay=0);

    // information
    void show(); // show timings
    
    // operations
    void start(); // start the run
    boolean running(); // returns false if we're done
    void stop(); // halt any time

  private:
    byte pin, off, on, cyclesTotal, remainingCycles;

    unsigned long onDuration, offDuration, startDelay;

    Metro timer;

    boolean isOn;

    void setOff();
    void setOn();
    void toggle();
};

#endif
