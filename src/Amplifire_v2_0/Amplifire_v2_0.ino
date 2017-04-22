// Adafruit HUZZAH ESP8266

#include <Streaming.h>
#include <Metro.h>
#include <Bounce.h>
#include <EEPROM.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#include "Solenoid.h"
Solenoid solenoid;
#define SOLENOID_PIN 15
#define SOLENOID_OFF LOW

#include "Sensor.h"
Sensor sensor;
#define ANALOG_PIN A0

#include "Server.h"
AP ap;

unsigned long simulationDelay = 5000UL;

#define NUM_LEDS 20
#define PIN_DATA 12 // yellow wire on LED strip
#define PIN_UNUSED 13 // pin is available on the 3pin screw terminal
CRGB leds[NUM_LEDS];
#define MASTER_BRIGHTNESS 255

// saved to EEPROM
struct Settings {
  unsigned long onDuration;
  unsigned long offDuration;
  byte nCycles;
  boolean armed;

  byte thresholdPercent;
  unsigned long retriggerDelay;
};
Settings s;

Metro simulationInterval(1UL);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // http://esp8266.github.io/Arduino/versions/2.0.0/doc/libraries.html#eeprom
  EEPROM.begin(512);
  loadEEPROM();
  s.armed = false; // just in case

  // set up the sensor
  sensor.begin(ANALOG_PIN);
  setSensorAction();

  // set up the solenoid
  solenoid.begin(SOLENOID_PIN, SOLENOID_OFF);
  setSolenoidAction();

  // set the simulation up
  setSimulationAction();

  Serial << endl;

  // set up the AP and web server
  ap.begin();
  ap.set(s.armed, s.onDuration, s.offDuration, s.nCycles, s.retriggerDelay, s.thresholdPercent);

  // set up the LEDs
  FastLED.addLeds<WS2811, PIN_DATA, RGB>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);

  // set master brightness control
  FastLED.setBrightness(MASTER_BRIGHTNESS);

}

void saveEEPROM() {
  // NEVER save armed=true
  boolean armedState = s.armed;
  s.armed = false;

  // push settings to EEPROM for power-up recovery
  EEPROM.put(0, s);
  EEPROM.commit();

  s.armed = armedState;
}

void loadEEPROM() {
  EEPROM.get(0, s);
  // just in case
  s.armed = false;
}

void loop() {
  // check for traffic
  if( ap.update() ) {
    // First, Do No Harm.
    solenoid.stop();

    // got traffic
    ap.get(s.armed, s.onDuration, s.offDuration, s.nCycles, s.retriggerDelay, s.thresholdPercent);

    // update settings
    setSolenoidAction();
    setSimulationAction();
    setSensorAction();

    // save 
    saveEEPROM();

    // probably another web request inbound, so go again
    return;
  }
  
  // see if arming state has changed
  if ( ! s.armed ) {

    // see if we need to run another trial loop
    if ( !solenoid.running() ) {
      if ( simulationInterval.check() ) {
        solenoid.start();
      }
    } else {
      simulationInterval.reset();
    }

  } else {
    // check for sensor activity
    if ( !solenoid.running() ) {
      if ( sensor.analogTrue() )
        solenoid.start();
    }
  }

  // update the lights
  showSettings(s.armed, solenoid.isFiring());

/*
  static Metro printInterval(1000UL);
  if ( printInterval.check() ) {
    sensor.show();
    solenoid.show();
    Serial << endl;
  }
*/
}

void showSettings(boolean armed, boolean on) {
  // used colors
  const CRGB armedOn = CRGB(255, 0, 0);
  const CRGB armedOff = CRGB(64, 0, 0);
  const CRGB disarmedOn = CRGB(0, 0, 255);
  const CRGB disarmedOff = CRGB(0, 0, 64);

  // track color
  static CRGB color = disarmedOff;

  if (armed && on) color = blend(color, armedOn, 50);
  else if (armed && !on) color = blend(color, armedOff, 50);
  else if (!armed && on) color = blend(color, disarmedOn, 5);
  else if (!armed && !on) color = blend(color, disarmedOff, 5);

  fill_solid(leds, NUM_LEDS, color);

  FastLED.show();
}

void setSimulationAction() {
  simulationInterval.interval( s.retriggerDelay );
}

void setSensorAction() {
  sensor.setThreshold(s.thresholdPercent);
  sensor.setRetriggerDelay(s.retriggerDelay + (s.onDuration + s.offDuration)*s.nCycles);

  sensor.show();
}

void setSolenoidAction() {
  solenoid.set(s.onDuration, s.offDuration, s.nCycles);
  solenoid.stop();
  if ( s.armed ) solenoid.arm();
  else solenoid.disarm();

  solenoid.show();
}

