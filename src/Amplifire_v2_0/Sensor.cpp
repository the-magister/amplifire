#include "Sensor.h"

void Sensor::begin(byte analogPin) {
  Serial << F("Sensor.  begin.");

  // set the pins
  this->analogPin = analogPin;
  pinMode( analogPin, INPUT);
  Serial << F("\tanalogPin=A") << this->analogPin;

  // get threshold value from EEPROM
  EEPROM.get(EEPROM_THRESH_LOC, this->analogThreshold);
  Serial << F("\tanalogThreshold=") << this->analogThreshold;

  Serial << endl;
}

void Sensor::setThreshold(byte percentThreshold) {

  // set our threshold from the average of 10 current readings.
  unsigned long readings = 0;
  for ( byte i = 0; i < 10; i++ ) {
    readings += this->analogValue();
    delay(5);
  }
  readings /= 10;
  
  // readings need to drop below X% of the current value for a trigger to happen
  this->analogThreshold = map(constrain(percentThreshold,0,100), 0, 100, 0, readings);

  Serial << F("Sensor.  setting analogThreshold=") << this->analogThreshold << endl;
}


// information
void Sensor::show() {
  Serial << F("Sensor: (") << analogValue() << F("/") << analogThreshold << F("->") << analogTrue() << F(") ");
  Serial << F("retrigger delay=") << this->retriggerDelay << F(". ");
}

void Sensor::setRetriggerDelay(unsigned long retriggerDelay) {
  this->retriggerDelay = retriggerDelay;
}


word Sensor::analogValue() {
  return ( analogRead(this->analogPin) );
}

// convenience functions
boolean Sensor::analogTrue() {
  
  boolean trigger = analogValue() <= this->analogThreshold;
  if( ! trigger ) return(false);
  
  boolean postDelay = (millis() - lastTriggerTime) > retriggerDelay;

  if ( trigger && postDelay ) {
    lastTriggerTime = millis();
    return ( true );
  } else {
    return ( false );
  }
}
