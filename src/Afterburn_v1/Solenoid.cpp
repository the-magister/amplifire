#include "Solenoid.h"

void Solenoid::setOff() { 
  this->isOn = false;
  digitalWrite(this->pin, this->off);
}
void Solenoid::setOn() { 
  this->isOn = true;;
  digitalWrite(this->pin, this->on);
}
void Solenoid::toggle() { 
  if( this->isOn ) {
    this->setOff();
  } else {
    this->setOn();
  }
}

void Solenoid::begin(byte pin, byte offDefinition) {
  this->pin = pin;
  this->off = offDefinition;
  this->on = !offDefinition;
  this->setOff();

  // then set the pin to OUTPUT _after_ setting to Off state!
  pinMode(this->pin, OUTPUT);

  Serial << F("Solenoid initializing.  pin=") << this->pin << F(" off=") << this->off << endl;

  this->set(0,0,0,0); // start with doing nothing.
}

void Solenoid::set(unsigned long onDuration, unsigned long offDuration, byte cycles, unsigned long startDelay) {
  this->cyclesTotal = constrain(cycles, 0, MAX_CYCLES);
  this->onDuration = constrain(onDuration, MIN_DURATION, MAX_DURATION);
  this->offDuration = constrain(offDuration, MIN_DURATION, MAX_DURATION);
  this->startDelay = constrain(startDelay, MIN_DURATION, MAX_DURATION);
  this->remainingCycles = 0;
}

void Solenoid::show() {
  Serial << F("Solenoid settings.  pin=") << this->pin << F(". on/off=") << this->onDuration << "/";
  Serial << this->offDuration << F(" ms. cycles=") << this->cyclesTotal << F(". start delay=") << this->startDelay; 
  Serial << " ms." << endl; 
}

void Solenoid::start() {
  // assign the work load.
  this->remainingCycles = this->cyclesTotal;
  this->setOff();
  
  if( startDelay> 0 ) {
    // we need to add some delay here
    this->timer.interval(startDelay);
    this->timer.reset();
  } else {
    // so will go on ASAP
    this->timer.interval(0);
    this->timer.reset();
  }
}

boolean Solenoid::running() {
  // bail out if we're not needed
  if( this->remainingCycles < 1 ) return(false);
   
  // is the timer up?
  if( this->timer.check() ) {
    this->toggle();
    if( this->isOn ) {
      // started a new cycle
      this->timer.interval(this->onDuration);
    } else {
      // ended a cycle
      this->timer.interval(this->offDuration);
      // decrement work
      this->remainingCycles--;
    }
    this->timer.reset();
  }
  
  return(true);
}

void Solenoid::stop() {
  this->remainingCycles = 0;
  this->setOff();
}

