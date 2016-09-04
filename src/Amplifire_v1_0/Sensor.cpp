#include "Sensor.h"

void Sensor::begin(byte analogPin, byte digitalPin, byte gndPin, byte vccPin) {
  Serial << F("Sensor.  begin.");

  // set the pins
  this->analogPin = analogPin;
  Serial << F("\tanalogPin=A") << this->analogPin;
  
  this->digitalPin = digitalPin;
  pinMode(digitalPin, INPUT);
  Serial << F("\tdigitalPin=") << this->digitalPin;
  
  // get threshold value from EEPROM
  EEPROM.get(EEPROM_THRESH_LOC, this->analogThreshold);
  Serial << F("\tanalogThreshold=") << this->analogThreshold;

  digitalWrite(gndPin, LOW);
  pinMode(gndPin, OUTPUT);
  Serial << F("\tgndPin=") << gndPin;
  
  digitalWrite(vccPin, HIGH);
  pinMode(vccPin, OUTPUT);
  Serial << F("\tvccPin=") << vccPin;

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
//  Serial << F("\tdigital=") << digitalValue();
  Serial << F("\tanalog=") << analogValue();
  Serial << F("\tthreshold=") << analogThreshold;
  Serial << F("\tanalogTrue=") << analogTrue();
//  Serial << F("\teitherTrue=") << eitherTrue();
//  Serial << F("\tbothTrue=") << bothTrue();
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
  return( digitalValue() == 0 );
}
boolean Sensor::analogTrue() {
  return( analogValue() <= this->analogThreshold );
}
boolean Sensor::eitherTrue() {
  return( digitalTrue() || analogTrue() );
}
boolean Sensor::bothTrue() {
  return( digitalTrue() && analogTrue() );
}

