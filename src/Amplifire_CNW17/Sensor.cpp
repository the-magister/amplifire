#include "Sensor.h"

void Sensor::begin(byte analogPin) {
  Serial << F("Sensor.  begin.");

  // set the pins
  this->analogPin = analogPin;
  pinMode( analogPin, INPUT);
  Serial << F("\tanalogPin=A") << this->analogPin;

  Serial << endl;

  currentReading = analogRead(this->analogPin); // start reasonable so we don't get a false trigger
}

void Sensor::setThreshold(byte percentThreshold) {

  // readings need to drop below X% of the current value for a trigger to happen
  this->analogThreshold = map(constrain(percentThreshold,0,100), 0, 100, 0, currentReading);

  Serial << F("Sensor: readings=") << currentReading << F(" *0.") << percentThreshold << F(" -> analogThreshold=") << this->analogThreshold << endl;
}


// information

void Sensor::show() {
  Serial << F("Sensor.");
  Serial << F(" triggered: ") << (currentReading <= this->analogThreshold);
  Serial << F(" threshold: ") << analogThreshold;
  Serial << F(" currentReading: ") << currentReading;
  Serial << F(" retrigger delay=") << this->retriggerDelay << F("ms. ");
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
  
  const unsigned long Nshort = 3;

  word current = analogRead(this->analogPin);

  currentReading = ((Nshort-1)*currentReading + current)/Nshort;

  updateEvery.reset();
}

// convenience functions
boolean Sensor::isTriggered() {
  
  boolean trigger = currentReading <= this->analogThreshold;
  if( ! trigger ) return(false);
  
  boolean postDelay = (millis() - lastTriggerTime) > retriggerDelay;

  if ( trigger && postDelay ) {
    lastTriggerTime = millis();
    return ( true );
  } else {
    return ( false );
  }
}
