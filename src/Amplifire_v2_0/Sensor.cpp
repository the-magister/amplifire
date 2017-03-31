#include "Sensor.h"

void Sensor::begin(byte analogPin) {
  Serial << F("Sensor.  begin.");

  // set the pins
  this->analogPin = analogPin;
  pinMode( analogPin, INPUT);
  Serial << F("\tanalogPin=A") << this->analogPin;

  Serial << endl;
}

void Sensor::setThreshold(byte percentThreshold) {

  // set our threshold from the average of 10ms of readings
  unsigned long readings = analogValue(10UL);
  
  // readings need to drop below X% of the current value for a trigger to happen
  this->analogThreshold = map(constrain(percentThreshold,0,100), 0, 100, 0, readings);

  Serial << F("Sensor: readings=") << readings << F(" *0.") << percentThreshold << F(" -> analogThreshold=") << this->analogThreshold << endl;
}


// information
void Sensor::show() {
  Serial << F("Sensor: (") << analogValue() << F("/") << analogThreshold << F("->") << analogTrue() << F(") ");
  Serial << F("retrigger delay=") << this->retriggerDelay << F("ms. ");
  Serial << endl;
}

void Sensor::setRetriggerDelay(unsigned long retriggerDelay) {
  this->retriggerDelay = retriggerDelay;
}


word Sensor::analogValue(unsigned long readFor) {
  Metro readTime(readFor);
  readTime.reset();
  // get 92 readings in 10ms, so ~0.1 ms/reading
  unsigned long values = 0;
  word count = 0;
  while( !readTime.check() ) {
    values += analogRead(this->analogPin);
    count++;
  }
  values /= count;
//  Serial << F("Sensor: ") << count << F(" readings and average value=") << values << endl;
  return ( values );
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
