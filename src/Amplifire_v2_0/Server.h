#ifndef Server_h
#define Server_h

#include <Arduino.h>
#include <Streaming.h>
#include <Metro.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DNSServer.h>

// handled outside of class
void handleRoot();
void handleCaptiveGateway();

class AP {
  public:
    // setup
    void begin(); // startup
  
    // call frequently to process requests; returns true if there's a change
    boolean update();

    // set values
    void set(boolean armed, unsigned long onDuration, unsigned long offDuration, byte nCycles, unsigned long retriggerDelay, byte thresholdPercent);

    // get values
    void get(boolean &armed, unsigned long &onDuration, unsigned long &offDuration, byte &nCycles, unsigned long &retriggerDelay, byte &thresholdPercent);

    // set out the web form
    void returnForm();
    
  private:
    String radioInput(String name, String value, boolean checked, String text);
    String numberInput(String name, int value, int minval, int maxval);
    
    boolean haveUpdate;

    boolean armed;
    unsigned long onDuration, offDuration, retriggerDelay;
    byte nCycles, thresholdPercent;
};

#endif
