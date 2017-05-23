#include "Solenoid.h"

void Solenoid::setOff() { 
  this->isOn = false;
  digitalWrite(this->pin, this->off);
}
void Solenoid::setOn() { 
  this->isOn = true;
  if( this->isArmed ) digitalWrite(this->pin, this->on);
}
void Solenoid::purge() {
  setOn();
  delay(3000);
  setOff();
}
void Solenoid::toggle() { 
  if( this->isOn ) {
    this->setOff();
  } else {
    this->setOn();
  }
}
boolean Solenoid::isFiring() {
  return( isOn );
}

void Solenoid::arm() { // allow firing of the solenoid
  if( !this->isArmed ) {
    Serial << F("**** ARMED ****") << endl;
    this->isArmed = true;
  }
}
void Solenoid::disarm() { // disallow firing of the solenoid.
  if( this->isArmed ) {
    Serial << F("** disharmed **") << endl;
    this->isArmed = false;
  }
}

void Solenoid::begin(byte pin, byte offDefinition) {
  this->pin = pin;
  this->off = offDefinition;
  this->on = !offDefinition;
  this->setOff();
  this->disarm();

  // then set the pin to OUTPUT _after_ setting to Off state!
  pinMode(this->pin, OUTPUT);

  Serial << F("Solenoid initializing.  pin=") << this->pin << F(" off=") << this->off << endl;

  this->set(0,0,0); // start with doing nothing.
}

void Solenoid::set(unsigned long onDuration, unsigned long offDuration, byte cycles) {
  this->cyclesTotal = constrain(cycles, 0, MAX_CYCLES);
  this->onDuration = constrain(onDuration, MIN_DURATION, MAX_DURATION);
  this->offDuration = constrain(offDuration, MIN_DURATION, MAX_DURATION);
  this->remainingCycles = 0;
}

void Solenoid::show() {
  Serial << F("Solenoid: ");  
  Serial << F("armed=") << isArmed << F(" ");
  Serial << F("on=") << onDuration << F("ms ");
  Serial << F("off=") << offDuration << F("ms ");
  Serial << F("cycles=") << cyclesTotal << F(". ");
  Serial << endl;
}

void Solenoid::start() {
  // assign the work load.
  this->remainingCycles = this->cyclesTotal;
  this->setOff();
  this->timer.interval(0);
  this->timer.reset();
}

boolean Solenoid::running() {
  // bail out if we're not needed
  if( this->remainingCycles <= 0 ) {
    this->setOff();
    return(false);
  } 

  // is the timer up?
  else if( this->timer.check() ) {
    
    if( this->remainingCycles <= 0) this->setOff();
    else this->toggle();
    
    if( this->isOn ) {
      // started a cycle
      
      this->timer.interval(this->onDuration);
    } else {
      // ended a cycle
      
      // decrement work
      this->remainingCycles--; 

      this->timer.interval(this->offDuration);
    }
    this->timer.reset();
  }
  
  return(true);
}

void Solenoid::stop() {
  // do not enforce cooldown if we're stopped this way
  this->setOff();
  this->remainingCycles = 0;
}

