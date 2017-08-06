/*
  WiFi Web Server LED Blink

 A simple web server and AP that lets you connect to the device via device AP and
 logon to your network wifi with HTTP /GET.
 /GET /pwid:password%ssid

 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 created Aug 2017
 by Ang Li
 */
#include <SPI.h>
#include <WiFi.h>

String ssid = "L4T-STAGE";      //  your network SSID (name)
String pass = "Passl4twifi1@#";   // your network password
String dssid = "irigyv1";      //  your network SSID (name)
String dpass = "irigyv12";   // your network password

int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  Serial.begin(9600);      // initialize serial communication
  pinMode(2, OUTPUT);      // set the LED pin mode

  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(dssid.c_str(), dpass.c_str());


  server.begin();                           // start the web server on port 80

}


void loop() {
  WiFiClient clientAP = server.available();   // listen for incoming clients
  //Start AP server and print out all device data
  if (clientAP) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    String Data = "";                    //data of full request
    String getData = "";
    Data = readRequest(currentLine, getData, clientAP);
    // close the connection:
    Serial.println("GET data request: ");
    //clientAP.stop();
    //Serial.println("client disonnected");
  }

  
}





//Functions

void wifiConnection() {
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid.c_str(), pass.c_str());
    // wait 10 seconds for connection:
    delay(10000);
  }
}

String readRequest (String currentLine, String GetData, WiFiClient client){
  
  while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("DATA 1<br>");
            client.print("DATA 2<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          //Serial.println(c);
          currentLine += c;      // add it to the end of the currentLine
          currentLine.trim();
          //if the current line ends with HTTP/1.1 GetData
          if (currentLine.startsWith("GET") && currentLine.endsWith("HTTP/")) {
            //Retrieve the GET data
            String lastData = GetData;
            GetData = currentLine;
            GetData.remove(0,4);
            GetData.remove(GetData.indexOf("HTTP/"));

            if (GetData.endsWith(".ico") || GetData.endsWith(".txt")) {
            GetData = lastData;
            } 
            
            return GetData;
            Serial.println(GetData);
            
            
            }
            
        }
      }
  }
} 

