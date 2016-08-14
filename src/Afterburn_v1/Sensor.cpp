#include "Sensor.h"

void Sensor::begin(byte analogPin, byte digitalPin) {
  Serial << F("Sensor.  begin.");

  // set the pins
  this->analogPin = analogPin;
  pinMode(analogPin, INPUT);
  Serial << F("\tanalogPin=") << this->analogPin;
  
  this->digitalPin = digitalPin;
  pinMode(digitalPin, INPUT);
  Serial << F("\tdigitalPin=") << this->digitalPin;
  
  // get threshold value from EEPROM
  EEPROM.get(EEPROM_THRESH_LOC, this->analogThreshold);
  Serial << F("\tanalogThreshold=") << this->analogThreshold;

  Serial << endl;
}

void Sensor::setThreshold(word analogThreshold) {
  this->analogThreshold = constrain(analogThreshold, 0, 1023);
  EEPROM.put(EEPROM_THRESH_LOC, this->analogThreshold);

  Serial << F("Sensor.  analogThreshold=") << this->analogThreshold << endl;
}

// information
void Sensor::show() {
  Serial << F("Sensor.");
  Serial << F("\tdigital=") << digitalValue();
  Serial << F("\tanalog=") << analogValue();
  Serial << F("\tthreshold=") << analogThreshold;
  Serial << F("\tanalogTruel=") << analogTrue();
  Serial << F("\teitherTrue=") << eitherTrue();
  Serial << F("\tbothTrue=") << bothTrue();
  Serial << endl;
}

// operations
byte Sensor::digitalValue() {
  return( digitalRead(this->digitalPin) );
}
word Sensor::analogValue() {
  return( analogRead(this->analogPin) );
}

// convenience functions
boolean Sensor::digitalTrue() {
  return( digitalValue() == 1 );
}
boolean Sensor::analogTrue() {
  return( analogValue() >= this->analogThreshold );
}
boolean Sensor::eitherTrue() {
  return( digitalTrue() || analogTrue() );
}
boolean Sensor::bothTrue() {
  return( digitalTrue() && analogTrue() );
}

