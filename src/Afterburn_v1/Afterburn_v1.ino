#include <Streaming.h>
#include <Metro.h>
#include <Bounce.h>
#include <EEPROM.h>
#include "Solenoid.h"
#include "Sensor.h"

Solenoid solenoid;
#define SOLENOID_PIN 3
#define SOLENOID_LOW LOW
#define FIRE_AGAIN_LOCKOUT_DURATION 500UL // after firing, don't fire again until this duration elapses.

Sensor sensor;
#define ANALOG_PIN A0
#define DIGITAL_PIN 11

#define MODE_PIN 5
Bounce modeSelect(MODE_PIN, 5UL);
const byte nModes = 2;
const unsigned long onDuration[nModes] = {50, 100};
const unsigned long offDuration[nModes] = {100, 50};
const byte nCycles[nModes] = {1, 5};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // set up the solenoid
  solenoid.begin(SOLENOID_PIN, SOLENOID_LOW);
  setMode(0);
  
  // set up the sensor
  sensor.begin(ANALOG_PIN, DIGITAL_PIN);
}

void loop() {

  // if we're firing, just pay attention to that
  static boolean haveJustFired = false;
  static Metro fireAgainLockout(FIRE_AGAIN_LOCKOUT_DURATION);
  if( solenoid.running() ) {
    haveJustFired = true;
    fireAgainLockout.reset();
    return;
  }

  // if we've just fired, check lockout timer to prevent refiring 
  if( haveJustFired ) {
    if( fireAgainLockout.check() ) {
      // ok to fire again
      haveJustFired = false;
    } else {
      return;
    }
  }

  // check for a mode change
  checkForModeSelect();

  // see if a new analog threshold has been given over serial
  checkForThresholdSet();

  // check for sensor activity
  checkForSensor();
}

void checkForModeSelect() {
  static byte currentMode = 0; // will set to zero at first run
  if( modeSelect.update() ) {
    // button state changed
    currentMode++;
    if( currentMode >= nModes ) currentMode=0;
    // change timings
    setMode(currentMode);
  }
}

void setMode(byte i) {
  solenoid.set(onDuration[i], offDuration[i], nCycles[i]);
  solenoid.show();
}

void checkForThresholdSet() {
  if( Serial.available() ) {
    delay(100UL); // wait for the rest of the input
    word analogThreshold = constrain(Serial.parseInt(), 0, 1023);
    sensor.setThreshold(analogThreshold);
  }
}

void checkForSensor() {
  static Metro printInterval(1000UL);
  if( printInterval.check() ) sensor.show();

  if( sensor.bothTrue() ) {
    Serial << F("*** FIRING ***") << endl;
    sensor.show();
    solenoid.start();
  }
}

