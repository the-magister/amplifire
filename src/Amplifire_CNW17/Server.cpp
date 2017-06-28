#include "Server.h"

ESP8266WebServer webServer(80);
String message;

void AP::begin(IPAddress myIP, String myName) {
  /* Set these to your desired credentials. */
  const char *ssid = "Amplifire";
  const char *password = "safetythird";
  const float powerSetting = 20.5; // 0 min, 20.5 max.  
  
  // naming
  this->unitName = myName;
  
  /* Soft AP network parameters */
  IPAddress dns(10,10,10,254);
  IPAddress gate(10,10,10,254);
  IPAddress netMsk(255,255,255,0);

  // set STA mode
  WiFi.disconnect();
  WiFi.setOutputPower(powerSetting); // max
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_STA);
  WiFi.config(myIP, dns, gate, netMsk);
  WiFi.begin(ssid, password);

  // Wait for connection
  Serial << F("Connecting to AP....");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial << endl;

  Serial << F("Server: connect to http://") << myIP << endl;

  delay(500); // Pause ?

//  webServer.setNoDelay(true); // ?
  webServer.on("/purge", handlePurge);
  webServer.on("/", handleRoot);
//  webServer.onNotFound( handleCaptiveGateway );
  webServer.begin();
  Serial << F("Server: web server started") << endl;

  // big strings, so let's avoid fragmentation by pre-allocating
  message.reserve(8192);
}


boolean AP::update() {
  this->haveUpdate = false;

  webServer.handleClient();

  static byte lastClient, lastStatus, lastStationCount;

  // Do NOT call this .client() method!!!!!
  //  byte client = webServer.client();
  
  byte status = WiFi.status();

  //  if( lastClient != client ) {
  //    Serial << F("Server: client change ") << lastClient << F("->") << client << endl;
  //  }
  if ( lastStatus != status ) {
    Serial << F("Server: WiFi status change ") << lastStatus << F("->") << status << endl;
    lastStatus = status;
  }

  return ( this->haveUpdate );
}
/*
   typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_DISCONNECTED     = 6
  } wl_status_t;
*/

void AP::set(boolean armed, unsigned long onDuration, unsigned long offDuration, byte nCycles, unsigned long retriggerDelay, byte thresholdPercent) {
  this->armed = armed;
  this->onDuration = onDuration;
  this->offDuration = offDuration;
  this->nCycles = nCycles;
  this->retriggerDelay = retriggerDelay;
  this->thresholdPercent = thresholdPercent;

  // signal to the main code that something has happened.
  this->haveUpdate = true;
}

void AP::get(boolean &armed, unsigned long &onDuration, unsigned long &offDuration, byte &nCycles, unsigned long &retriggerDelay, byte &thresholdPercent) {
  armed = this->armed;
  onDuration = this->onDuration;
  offDuration = this->offDuration;
  nCycles = this->nCycles;
  retriggerDelay = this->retriggerDelay;
  thresholdPercent = this->thresholdPercent;
}


const char HEAD_FORM[] PROGMEM =
  "<!DOCTYPE HTML>"
  "<html>"
  "<head>"
  //  "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
  "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0\">"
  "<title>Amplifire</title>"
  "<style>"
  "\"body { background-color: #000000; font-family: Arial, Helvetica, Sans-Serif; Color: #808080; }\""
  "</style>"
  "</head>"
  "<body>"
  "<h2>Amplifire</h2>"
  ;

const char TAIL_FORM[] PROGMEM =
  "</body>"
  "</html>"
  ;


extern AP ap;

void handleCaptiveGateway() {
  // append header
  message = FPSTR(HEAD_FORM);

  message += "<h2><a href=\"http://192.168.1.1/\">http://192.168.1.1/</a></h2>";

  // append footer
  message += FPSTR(TAIL_FORM);

  // send form
  webServer.send(200, "text/html", message);


}

void handlePurge() {
  Serial << F("Server: purge request.") << endl;
  solenoid.purge();
  
  ap.returnForm();
}

void handleRoot() {
  Serial << F("Server: client request.") << endl;
  
  if (webServer.hasArg("Armed")) {
    Serial << F("Server: responding to form imputs.") << endl;
  
    // process submission
    ap.set(
      webServer.arg("Armed").toInt(),
      webServer.arg("onDuration").toInt(),
      webServer.arg("offDuration").toInt(),
      webServer.arg("nCycles").toInt(),
      webServer.arg("retriggerDelay").toInt(),
      webServer.arg("thresholdPercent").toInt()
    );
  }

  // send back the form
  ap.returnForm();
  Serial << F("Server: sent form.") << endl;
}

void AP::returnForm() {
  // append header
  message = FPSTR(HEAD_FORM);

  // open form
  message += "<FORM action=\"/\" method=\"post\">";
  message += "<P>";

  message += "Unit: <em>" + this->unitName + "</em>";
  // form contents
  message += "<h4>Arming</h4>";
  message += "Armed: ";
  message += radioInput("Armed", "1", false, "ARMED"); // Note that we never default to "ARMED" setting, even if that was pressed last time.
  message += radioInput("Armed", "0", true, "Disarmed"); // Note that we never default to "ARMED" setting, even if that was pressed last time.
  message += "<br>";

  message += "<h4>Trigger</h4>";
  message += "Trigger Sensitivity [0-90%]: ";
  message += numberInput("thresholdPercent", this->thresholdPercent, 0, 90);
  message += "<br>Retrigger After [50-3000] ms: ";
  message += numberInput("retriggerDelay", this->retriggerDelay, 50, 3000);
  message += "<br>";

  message += "<h4>Timing</h4>";
  message += "Open Time [50-3000] ms: ";
  message += numberInput("onDuration", this->onDuration, 50, 3000);
  message += "<br>Close Time [50-3000] ms: ";
  message += numberInput("offDuration", this->offDuration, 50, 3000);
  message += "<br>Number of open-close cycles: ";
  message += numberInput("nCycles", this->nCycles, 1, 10);
  message += "<br><br>";

  // close form
  message += "<INPUT type=\"submit\" value=\"Go!\">";
  message += "</P>";
  message += "</FORM>";

  // append footer
  message += FPSTR(TAIL_FORM);

  // send form
  webServer.send(200, "text/html", message);

}


// helper functions to construct a web form
String AP::radioInput(String name, String value, boolean checked, String text) {
  String Checked = checked ? "checked" : "";

  return ( "<INPUT type=\"radio\" name=\"" + name + "\" value=\"" + value + "\" " + Checked + ">" + text );
}
String AP::numberInput(String name, int value, int minval, int maxval) {
  return ( "<INPUT type=\"number\" name=\"" + name + "\" value=\"" + String(value) + "\" min=\"" + String(minval) + "\" max=\"" + String(maxval) + "\">");
}

