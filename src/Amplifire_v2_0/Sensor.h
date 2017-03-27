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
    void begin(byte pin); // set the pin

    // information
    void show(); // show current settings

    // operations
    void setRetriggerDelay(unsigned long retriggerDelay);
    void setThreshold(byte percentThreshold);

    // convenience functions
    word analogValue();
    boolean analogTrue();

  private:
    byte analogPin;
    word analogThreshold;
    unsigned long retriggerDelay;
    unsigned long lastTriggerTime;

};

#endif
