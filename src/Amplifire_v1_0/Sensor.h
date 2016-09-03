#ifndef Sensor_h
#define Sensor_h

#include <Arduino.h>
#include <Streaming.h>
#include <EEPROM.h>

#define EEPROM_THRESH_LOC 101

// For: http://www.sainsmart.com/ir-infrared-flame-detection-sensor-module-detect-fire-flame-sensor-for-arduino.html

class Sensor {
  public: 
    // setup
    void begin(byte analogPin=0, byte digitalPin=12, byte gndPin=11, byte vccPin=10); // set the pin
    void setThreshold(word analogThreshold);

    // information
    void show(); // show current settings
    
    // operations
    byte digitalValue();
    word analogValue();

    // convenience functions
    boolean digitalTrue();
    boolean analogTrue();
    boolean eitherTrue();
    boolean bothTrue();

  private:
    byte analogPin, digitalPin;
    word analogThreshold;

};

#endif
