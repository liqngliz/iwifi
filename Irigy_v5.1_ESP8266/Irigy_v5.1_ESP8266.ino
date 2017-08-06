/*
  WiFi Web Server and HTTP GET with simple string API

 A simple web server and AP that lets you connect to the device via device AP and
 logon to your network wifi with HTTP /GET.
 e.g /GET /pwid:password%ssid, /GET /dinfo:UDID%mode%etc,

 API fron request and response is returned by  
 
 String readRequest (WiFiClient client) returns the Get API
 String readResponse (WiFiClient client) returns the API response from server
 
 Get request/reponse API is key:data%data%data% and can be parsed to extract data as string using 
 parseDataKey (String data) to return key
 parseDataAsString (String data, int dataPosition) with dataPosition of first object as 1 to return data
  
 This example is written for a network using WPA encryption. For
 WEP or WPA, change the Wifi.begin() call accordingly.

 Built on ESP32, ESP8266
 Expected compatibility with all devices with WiFi.softAP().
 
 created Aug 2017
 by Ang Li
 */
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

String ssid = "Testing";      //  your network SSID (name)
String pass = "testing";   // your network password
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
  
 // wifiConnection();
 // printWifiStatus();
  server.begin();                           // start the web server on port 80

  //Serial.println(WiFi.softAPIP());

}


void loop() {
  WiFiClient clientAP = server.available();   // listen for incoming clients
  //Start AP server and print out all device data
  if (clientAP) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String Data = "";                       //Store data of GET request
    Data = readRequest(clientAP); //Parse the incoming request to get the GET request string
   
    // logon to Wifi
    Serial.println("Key: ");
    Serial.println (parseDataKey(Data));
    if (parseDataKey(Data).endsWith("pwssid")){
      
      Serial.println("new SSID: ");
      Serial.println (parseDataAsString(Data, 1));
      ssid = parseDataAsString(Data, 1);
      
      Serial.println("new Pass: ");
      Serial.println (parseDataAsString(Data, 2));
      pass = parseDataAsString(Data, 2);

      //Wifi connection
      wifiConnection();
      printWifiStatus();
    }

     // close the connection:
    clientAP.stop();
    Serial.println("client disonnected");
    
  }

}





//Functions
//Parse GET request data to get Key with no ":"
String parseDataKey (String data) {
  //Get the key of the substring Data
  return data.substring(0, data.indexOf(":"));
}

//Parse string to get data as a string, starts with 1 as position
String parseDataAsString (String data, int dataPosition){
  //we can parse up to 16 different string requests each data position is separated by a %
  //pwssid:Livebox-B15C%C2F59CD5699AF134E5E9145E4F
  String dataNoKey = data.substring(data.indexOf(":")+1);
  String currData = "";
  int n = 0;
  String returnData = "";
  char r;
  
  for (int i = 0; i < dataNoKey.length(); i++){
    r = dataNoKey[i];
    
    if (r == '%') {
      n = n + 1; 
      if (n == dataPosition) {
        return currData;
      }
      currData = "";      
    } else {
      currData += r; 
    }
    
    if( i == dataNoKey.length()){
      return currData;
    }    
  }
  
}

//Connect to Wifi
void wifiConnection() {
  // attempt to connect to Wifi network:
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid.c_str(), pass.c_str());
    // wait 10 seconds for connection:
    delay(10000);
  }
}

//Print Wifi Status
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//Read request and respond accordingly, output a string with the valid GET request 
String readRequest (WiFiClient client){
  
  String currentLine = "";                // make a String to hold incoming data from the client
  String GetData = "";                    //Hold incoming request
  
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
          }
            
        }
      }
  }
 return GetData; 
} 

