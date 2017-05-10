#include "Sensor.h"

void Sensor::begin(byte analogPin) {
  Serial << F("Sensor.  begin.");

  // set the pins
  this->analogPin = analogPin;
  pinMode( analogPin, INPUT);
  Serial << F("\tanalogPin=A") << this->analogPin;

  Serial << endl;

  averageReading = 1024; // start high so we don't get a false trigger
}

void Sensor::setThreshold(byte percentThreshold) {

  // readings need to drop below X% of the current value for a trigger to happen
  this->analogThreshold = map(constrain(percentThreshold,0,100), 0, 100, 0, averageReading);

  Serial << F("Sensor: readings=") << averageReading << F(" *0.") << percentThreshold << F(" -> analogThreshold=") << this->analogThreshold << endl;
}


// information
void Sensor::show() {
  Serial << F("Sensor: (") << averageReading << F("/") << analogThreshold << F("->") << isTriggered() << F(") ");
  Serial << F("retrigger delay=") << this->retriggerDelay << F("ms. ");
  Serial << endl;
}

void Sensor::setRetriggerDelay(unsigned long retriggerDelay) {
  this->retriggerDelay = retriggerDelay;
}


void Sensor::update() {
  // the internet tells me that the WiFi device is using the ADC to adjust power output,
  // so I'm only using analogRead every interval.
  static Metro updateEvery(10UL);
  if (!updateEvery.check() ) return;
  
  const unsigned long N = 3;

  word current = analogRead(this->analogPin);

  averageReading = ((N-1)*averageReading + current)/N;

  updateEvery.reset();
}

// convenience functions
boolean Sensor::isTriggered() {
  
  boolean trigger = averageReading <= this->analogThreshold;
  if( ! trigger ) return(false);
  
  boolean postDelay = (millis() - lastTriggerTime) > retriggerDelay;

  if ( trigger && postDelay ) {
    lastTriggerTime = millis();
    return ( true );
  } else {
    return ( false );
  }
}
