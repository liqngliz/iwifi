
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
#include <WiFi.h>
#include <elapsedMillis.h>

//Device Parameters
String DeviceID = "18c29d17-71ea-11e7-9c8c-e0db55fe78c8";
String WaterID = "18c29d31-71ea-11e7-9c8c-e0db55fe78c8";
int humlvl = 280;
int alert = 0;
int Mode = 0;
int HumMax = 324;
int HumMin = 100; 
int Quantity = 1500;  //in seconds
int WaterNow = 0;
int Timer = 12;       //in hours

elapsedMillis timeElapsed;
unsigned int interval = 20000;

//Connection parameters
String ssid = "Testing";      //  your network SSID (name)
String pass = "testing";   // your network password
String dssid = "irigyv1";      //  your network SSID (name)
String dpass = "irigyv12";   // your network password

const char* host = "www.oneviewdigital.com";


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

  //if not connected to wifi stop loop here
  if (status != WL_CONNECTED) {
    return;
  }
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  int httpPort = 80;
  String testRequest = "/?DeviceID=18c29d17-71ea-11e7-9c8c-e0db55fe78c8&humlvl=280&Alert=0&WateredID=18c29d31-71ea-11e7-9c8c-e0db55fe78c8";
  
  //make a request every 20 seconds.

  String resp = "";
  
  if (timeElapsed > interval) 
  {       
    resp = webServerRequest(client, httpPort, testRequest);
    Serial.println ("the resp is:");
    resp = serverResponse (resp);
    
    timeElapsed = 0;       // reset the counter to 0 so the counting starts over...
  }

  //get the key and request
  if (parseDataKey(resp).endsWith("mmxmnqtwnt")) {
    Serial.println("Key: ");
    Serial.println (parseDataKey(resp));
    
    Serial.println("Mode: ");
    Mode = parseDataAsString(resp, 1).toInt();
    Serial.println(Mode);

    Serial.println("HumMax: ");
    HumMax = parseDataAsString(resp, 2).toInt();
    Serial.println(HumMax);
    
    Serial.println("HumMin: ");
    HumMin = parseDataAsString(resp, 3).toInt();
    Serial.println(HumMin);

    Serial.println("Quantity: ");
    Quantity = parseDataAsString(resp, 4).toInt();
    Serial.println(Quantity);

    Serial.println("WaterNow: ");
    WaterNow = parseDataAsString(resp, 5).toInt();
    Serial.println(WaterNow);
    
    Serial.println("Timer: ");
    Timer = parseDataAsString(resp, 6).toInt();
    Serial.println(Timer);
  }
  
  client.stop();

}





//Functions

//parse server response

/*
<!doctype html>

<html lang="en">
<head>
  <meta charset="utf-8">
  <title>The HTML5 Herald</title>
</head>
<body>
  mmxmnqtwnt:0%324%100%1500%0%12
</body>
</html>

0*/

String serverResponse (String httpresponse) {
  
  httpresponse.remove(0, httpresponse.indexOf("<body>"));
  httpresponse.remove(0,7);
  httpresponse.remove(httpresponse.indexOf("</body>"));
  httpresponse.trim();
  Serial.println(httpresponse);
  return httpresponse;
}

//Send a Get request to the server
String webServerRequest (WiFiClient client, int httpPort, String request) {
  Serial.print("connecting to ");
  Serial.println(host);
  
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return "";
  }
  
  // We now create a URI for the request
  
  Serial.print("Requesting: ");
  Serial.println(request);
  
  // This will send the request to the server
  client.print(String("GET ") + request + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return "";
    }
  }
  String response = "";
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    char c = client.read();
    response += c;
  }

  Serial.print (response);
  response.trim();
  return response;
}


//Parse GET request data to get Key with no ":"
String parseDataKey (String data) {
  //Get the key of the substring Data
  return data.substring(0, data.indexOf(":"));
}

//Parse string to get data as a string, starts with 1 as position
String parseDataAsString (String data, int dataPosition){
  //we can parse up to 16 different string requests each data position is separated by a %
  //pwssid:L4T-STAGE%Passl4twifi1@%
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
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid.c_str(), pass.c_str());
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

