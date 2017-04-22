#include "Server.h"

DNSServer dnsServer;
ESP8266WebServer webServer(80);
String message;

void AP::begin() {
  /* Set these to your desired credentials. */
  const char *ssid = "Amplifire";
  const char *password = "safetythird";
  const char *myHostname = "amplifire";

  /* Soft AP network parameters */
  IPAddress apIP(192, 168, 1, 1);
  IPAddress gate(192, 168, 1, 1);
  IPAddress netMsk(255, 255, 255, 0);

  // set AP mode
  WiFi.setAutoConnect(false); // ?
  WiFi.setOutputPower(20.5); // max
  WiFi.disconnect();
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.mode(WIFI_AP);

  Serial << F("Server: configuring AP(") << ssid << F(") password(") << password << F(").") << endl;
  WiFi.softAPConfig(apIP, gate, netMsk);
  int channel = 8; // [1-13]
  int hidden = 0; // 0=broadcast SSID
  int max_conn = 1; // number of connections needed.
  WiFi.softAP(ssid, password, channel, hidden);
  delay(500); // Without delay I've seen the IP address blank

  IPAddress myIP = WiFi.softAPIP();
  Serial << F("Server: connect to http://") << myIP << endl;

  // what's up?
//  WiFi.printDiag( Serial ); 
  
  // Setup the DNS server redirecting all the domains to the apIP
  const byte DNS_PORT = 53;
  //  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  if ( dnsServer.start(DNS_PORT, "*", apIP) ) {
    Serial << F("Server: DNS started") << endl;
  } else {
    Serial << F("Server: ERROR on DNS startup!") << endl;
  }
  delay(500); // Pause ?

//  webServer.setNoDelay(true); // ?
  webServer.on("/", handleRoot);
//  webServer.onNotFound( handleCaptiveGateway );
  webServer.begin();
  Serial << F("Server: web server started") << endl;

  // big strings, so let's avoid fragmentation by pre-allocating
  message.reserve(8192);
}


boolean AP::update() {
  this->haveUpdate = false;

  dnsServer.processNextRequest();
  webServer.handleClient();

  static byte lastClient, lastStatus, lastStationCount;
  //  byte client = webServer.client();
  // Do NOT call this .client() method!!!!!
  byte status = WiFi.status();
  byte stationCount = WiFi.softAPgetStationNum();

  //  if( lastClient != client ) {
  //    Serial << F("Server: client change ") << lastClient << F("->") << client << endl;
  //  }
  if ( lastStatus != status ) {
    Serial << F("Server: WiFi status change ") << lastStatus << F("->") << status << endl;
    lastStatus = status;
  }
  if ( lastStationCount != stationCount ) {
    Serial << F("Server: AP station count change ") << lastStationCount << F("->") << stationCount << endl;
    lastStationCount = stationCount;
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

/*

  ESP8266WebServer server(80);

  server.handleClient();


  String message = "";
  // big strings, so let's avoid fragmentation by pre-allocating
  message.reserve(4096);


  void returnFail(String msg) {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(500, "text/plain", msg + "\r\n");
  }

  const char HEAD_FORM[] PROGMEM =
         client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 5");  // refresh the page automatically every 5 sec
          client.println();

  "<!DOCTYPE HTML>"
  "<html>"
  "<head>"
  "<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
  "<title>Amplifire</title>"
  "<style>"
  "\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
  "</style>"
  "</head>"
  "<body>"
  "<h2>Amplifire</h2>"
  ;

  const char TAIL_FORM[] PROGMEM =
  "</body>"
  "</html>"
  ;

  // helper functions to construct a web form
  String radioInput(String name, String value, boolean checked, String text) {
  String Checked = checked ? "checked" : "";

  return ( "<INPUT type=\"radio\" name=\"" + name + "\" value=\"" + value + "\" " + Checked + ">" + text );
  }
  String numberInput(String name, int value, int minval, int maxval) {
  return ( "<INPUT type=\"number\" name=\"" + name + "\" value=\"" + String(value) + "\" min=\"" + String(minval) + "\" max=\"" + String(maxval) + "\">");
  }

  void returnForm() {
  // append header
  message = FPSTR(HEAD_FORM);

  // dynamic form
  message += "<FORM action=\"/\" method=\"post\">";
  message += "<P>";

  message += "<h4>Controls</h4>";
  message += "Brightness ";
  message += numberInput("Brightness", s.bright, 0, 255);

  message += "  Sparkles ";
  message += numberInput("Sparkles", s.sparkles, 0, 255);

  message += "  Segments ";
  message += numberInput("Segments", s.segments, 1, MAX_SEGMENTS);

  message += "<h4>Color</h4>";
  message += radioInput("Color", "0", s.color == 0, "Off") + " ";
  message += radioInput("Color", "1", s.color == 1, "CheerLights") + " ";
  message += "<br><br>";

  message += "Whites: ";
  message += radioInput("Color", "12", s.color == 12, "FairyLight");
  message += radioInput("Color", "13", s.color == 13, "White");
  message += radioInput("Color", "14", s.color == 14, "Snow");
  message += "<br><br>";

  message += "CheerLights: ";
  message += radioInput("Color", "2", s.color == 2, "Red");
  message += radioInput("Color", "3", s.color == 3, "Green");
  message += radioInput("Color", "4", s.color == 4, "Blue");
  message += radioInput("Color", "5", s.color == 5, "Cyan");
  message += radioInput("Color", "6", s.color == 6, "OldLace");
  message += radioInput("Color", "7", s.color == 7, "Purple");
  message += radioInput("Color", "8", s.color == 8, "Magenta");
  message += radioInput("Color", "9", s.color == 9, "Yellow");
  message += radioInput("Color", "10", s.color == 10, "Orange");
  message += radioInput("Color", "11", s.color == 11, "Pink");
  message += "<br><br>";

  message += "For Patty: ";
  message += radioInput("Color", "15", s.color == 15, "DarkGreen");
  message += radioInput("Color", "16", s.color == 16, "Brown");
  message += radioInput("Color", "17", s.color == 17, "Crimson");
  message += radioInput("Color", "18", s.color == 18, "CornflowerBlue");
  message += radioInput("Color", "19", s.color == 19, "CadetBlue");
  message += radioInput("Color", "20", s.color == 20, "Indigo");
  message += radioInput("Color", "21", s.color == 21, "DeepPink");
  message += radioInput("Color", "22", s.color == 22, "LightCoral");
  message += radioInput("Color", "23", s.color == 23, "MediumVioletRed");
  message += radioInput("Color", "24", s.color == 24, "PaleVioletRed");
  message += radioInput("Color", "25", s.color == 25, "LightSlateGray");
  message += radioInput("Color", "26", s.color == 26, "Goldenrod");
  message += "<br><br>";

  message += "Palettes: ";
  message += radioInput("Color", "50", s.color == 50, "RedGreenWhite");
  message += radioInput("Color", "51", s.color == 51, "Holly");
  message += radioInput("Color", "52", s.color == 52, "RedWhite");
  message += radioInput("Color", "53", s.color == 53, "BlueWhite");
  message += radioInput("Color", "54", s.color == 54, "FairyLight");
  message += radioInput("Color", "55", s.color == 55, "Snow");
  message += radioInput("Color", "56", s.color == 56, "RetroC9");
  message += radioInput("Color", "57", s.color == 57, "Ice");
  message += "<br><br>";

  message += "<INPUT type=\"submit\" value=\"Update Lights\">";
  message += "</P>";
  message += "</FORM>";

  // append footer
  message += FPSTR(TAIL_FORM);

  // send form
  server.send(200, "text/html", message);

  //  Serial << F("Sent ") << message.length() << F(" bytes:") << message << endl;
  }

  void handleSubmit() {
  if (!server.hasArg("Color")) return returnFail("BAD ARGS");

  // get form results
  s.color = server.arg("Color").toInt();
  s.sparkles = server.arg("Sparkles").toInt();
  s.bright = server.arg("Brightness").toInt();
  s.segments = server.arg("Segments").toInt();

  // display
  Serial << F("Request: color=") << s.color << F(" sparkles=") << s.sparkles;
  Serial << F(" brightness=") << s.bright << F(" segments=") << s.segments << endl;

  updateFromSettings();
  }

  void handleNotFound() {
  message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  }



  if ( WiFi.status() == WL_CONNECTION_LOST ) {
    Serial << F("WiFi connection lost!") << endl;
  }

  if ( WiFi.status() == WL_DISCONNECTED ) {
    Serial << F("WiFi disconnected!") << endl;
  }

  //connect wifi if not connected
  if (WiFi.status() != WL_CONNECTED) {
    delay(50);
    WiFi.disconnect();
    delay(50);
    connect(); // and reconnect
    return;
  }


  void connect(void) {
  WiFiManager wifiManager;

  // will run the AP for three minutes and then try to reconnect
  wifiManager.setConfigPortalTimeout(180);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("TreeLightsAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("subnet mask: ");
  Serial.println(WiFi.subnetMask());
  Serial.print("gateway: ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());

  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);

  if ( MDNS.begin ( "treelights" ) ) {
    Serial.println ( "MDNS responder started" );
  }

  }



*/
