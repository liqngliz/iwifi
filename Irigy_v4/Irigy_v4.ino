#include <SPI.h>
#include <elapsedMillis.h>
#include <WiFi.h>


//ssidpw:L4T-STAGE%Passl4twifi1@# , ssidpw:Livebox-B15C%C2F59CD5699AF134E5E9145E4F
String ssid = "L4T-STAGE"; //  your network SSID (name)
String pass = "Passl4twifi1@#";    // your network password (use for WPA, or use as key for WEP)

String dssid = "IrigyV1";
String dpass = "WateringPlants";

int status = WL_IDLE_STATUS;

WiFiServer server(80);


//Device variables
String UDID;
int deviceMode = 0;
int humlvl = 0;
int hummax = 0;
int hummin = 0;
int qty = 1000;
int alert = 0;
int light = 0;
String UWID;

elapsedMillis timeElapsed;

void setup() {
//Setup
bool apinfo;
Serial.begin(9600);
while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
}
delay(1000);
//Setup Wifi and AP
Serial.println();
Serial.println("Setup");

IPAddress ip = WiFi.localIP();
Serial.print("IP Address: ");
Serial.println(ip);

//Setup soft AP and turn on WIFI AP & STA mode
apinfo = WiFi.softAP(dssid.c_str(), dpass.c_str(), 3, 0);
WiFi.mode(WIFI_AP_STA);

server.begin();

}

int value = 0;

void loop() {
  
  WiFiClient client = server.available() ;
  // if you get a client,
  if (client) {       
    Serial.println("New Client.");
    String currentLine = "";
    
    while (client.connected()) { 
     if (client.available()) {
      char c = client.read();
      Serial.write(c);
       client.println("HTTP/1.1 200 OK");
       client.println("Content-type:text/html");
       client.println();

       // the content of the HTTP response follows the header:
       client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
       client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

       // The HTTP response ends with another blank line:
       client.println();
     }
    }
   }
      
                      

  
  client.stop();

  WiFiClient clienthttp;

  clienthttp.stop();
}
