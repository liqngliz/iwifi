
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

 Written for ESP32
 
 created Aug 2017
 by Ang Li
 */
 /*
 TODO Implement SSL - DO NOT USE GODADDY SSL, only have intermediate SSL
 Symatec works
 Digicert may work (to be tested)
 */
 
#include <SPI.h>
#include <WiFi.h>
#include <elapsedMillis.h>
#include <Preferences.h>

//Pin Settings for sensors

int analogWaterPin = 32;
int waterlvl = 0; 

//Device Parameters
//DeviceID, WaterID, Mode, HumMax, HumMin, Quantity, Timer, dssid, ssid, WiFiConnected, extraData
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
int lightlvl = 0;
String extraData = "";
String WiFiConnected = "false";

//Device preferences store
Preferences preferences;

elapsedMillis timeElapsed;
elapsedMillis measureTimeElapsed;
unsigned short int interval = 20000;
unsigned short int measuredelay = 1000;

//Connection parameters
String ssid = "";      //  your network SSID (name)
String pass = "";   // your network password
String dssid = "irigy";      //  your network SSID (name)
String dpass = "irigy";   // your network password

const char* host = "www.irigapi.com";


int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
  //initiate Pins
  
  //Start serial communications
  Serial.begin(9600);      // initialize serial communication
  delay(500);
  //TODO Close valve
  
  Serial.println();
  Serial.println("Starting Irigy");
  
  //Start preferences RW mode
  preferences.begin("irigy", false);
  
  //Get Preferences for device settings before power off
  DeviceID = preferences.getString("DeviceID","18c29d17-71ea-11e7-9c8c-e0db55fe78c8");
  WaterID = preferences.getString("WaterID", "18c29d31-71ea-11e7-9c8c-e0db55fe78c8");
  
  Mode = preferences.getInt("Mode", 0);
  HumMax = preferences.getInt("HumMax", 324);
  HumMin = preferences.getInt("HumMin", 100);
  Quantity =  preferences.getInt("Quantity", 1500);
  Timer = preferences.getInt("Timer", 12);
  
  //Set variables for AP broadcast
  dssid = preferences.getString("dssid", "irigy");
  dpass = preferences.getString("dpass", "watermyplant");

  Serial.println("AP:");
  Serial.println(dssid);
  Serial.println(dpass);
  
  //Set SSID and Pass a for logon
  ssid = preferences.getString("ssid", "");
  pass = preferences.getString("pass", "");
  

  delay(500);
  WiFi.mode(WIFI_AP_STA);
  
  WiFi.softAP(dssid.c_str(), dpass.c_str());
  
  if (ssid.length() > 0) {
    wifiConnection();
    printWifiStatus();
  }
  Serial.println("Wifi:");
  Serial.println(ssid);
  Serial.println(pass);
  
  server.begin();                           // start the web server on port 80

  delay(1000);

}


void loop() {
  //User starts Irigy

  if (measureTimeElapsed > measuredelay) {
    //Read the water level
    waterlvl = 4095 - analogRead(analogWaterPin);
    humlvl = waterlvl;
    Serial.println(humlvl);
    //Read the light level
    
    //Reset timer for next second
    measureTimeElapsed = 0;
  }

  //TODO If humlvl < hummin water water until (humlvl-hummin) > hummax/2, if humlvl does not increase in 5 seconds issue alert not enough water
  //Then water until humlvl >= hummax/4 at rate of 1.5 seconds with 2 second delay
  //User can then choose to top off
  //add last watered in seconds
 
  //Save the current hum lvl in humlvl array of last 10 measurements
  //If the all 3 measurements are 0 do not open valve, only open if all 3 moisture readings are constant or if user starts
  //If the change between the last 7 measurements and last 2 measurements is greater than Minhum/2 and current humlvl < minhum, do not turn on
  //Ask if user has pulled out irigy

  //Check wifi status
  if (status != WL_CONNECTED) {
    WiFiConnected = "false";
  } else {
    WiFiConnected = "true";
  }
  
  WiFiClient clientAP = server.available();   // listen for incoming clients
  //Start AP server and print out all device data
  if (clientAP) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String Data = "";                       //Store data of GET request
    Data = readRequest(clientAP); //Parse the incoming request to get the GET request string

    //The response should not exceed 4 kilobyte, to prevent memory issues
    if (Data.length() > 4096) {
      return;
    }
    
    // logon to Wifi
    Serial.println("Key: ");
    Serial.println (parseDataKey(Data));
    if (parseDataKey(Data).endsWith("pwssid") && parseDataKey(Data).length() == 6){
      
      Serial.println("new SSID: ");
      Serial.println (parseDataAsString(Data, 1));
      ssid = parseDataAsString(Data, 1);
      
      Serial.println("new Pass: ");
      Serial.println (parseDataAsString(Data, 2));
      pass = parseDataAsString(Data, 2);
      
      preferences.putString("ssid", ssid);
      preferences.putString("pass", pass);
      
      Serial.println (preferences.getString("pass", ssid));
      
      //Wifi connection
      wifiConnection();
      printWifiStatus();
    } else if (parseDataKey(Data).endsWith("dpwssid") && parseDataKey(Data).length() == 7){
      //save SSID data AP
        dssid = parseDataAsString(Data, 1);
        dpass = parseDataAsString(Data, 2);
        preferences.putString("dssid", dssid);
        preferences.putString("dpass", dpass);
        Serial.println("AP Changed");
    }

    //Set the Device ID and Water ID
    //dwID:DeviceID%WaterID
    if (parseDataKey(Data).endsWith("dwID") && parseDataKey(Data).length() == 4) {
      DeviceID = parseDataAsString(Data, 1);
      WaterID = parseDataAsString(Data, 2);
      preferences.putString("DeviceID", DeviceID);
      preferences.putString("WaterID", WaterID);
      Serial.println("ID Changed");
    }
    
     // close the connection:
    clientAP.stop();
    Serial.println("client disonnected");

  }

  //Reset all variables on button press
  static uint8_t lastPinState = 1;
  uint8_t pinState = digitalRead(0);
  if(!pinState && lastPinState){
        Serial.println("Button Pressed");
        preferences.clear();
  }
    lastPinState = pinState;
  
  //if not connected to wifi stop loop here
  if (status != WL_CONNECTED) {
    return;
  }
  
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  int httpPort = 80;
  //Build request
  //String testRequest = "/?DeviceID=18c29d17-71ea-11e7-9c8c-e0db55fe78c8&humlvl=280&Alert=0&WateredID=18c29d31-71ea-11e7-9c8c-e0db55fe78c8";
  
  String Request = "/?DeviceID=" + DeviceID + "&humlvl=" + String(humlvl) + "&lightlvl=" + String(lightlvl) + "&Alert=" + String(alert) + "&WateredID=" + WaterID;
  
  //make a request every 20 seconds.

  String resp = "";
  
  if (timeElapsed > interval) 
  {       
    resp = webServerRequest(client, httpPort, Request);
    Serial.println ("the resp is:");
    resp = serverResponse (resp);
    
    //Response should be less than 2 kilobyes
    if (resp.length() > 4096) {
      return;
    }
    
    timeElapsed = 0;    // reset the counter to 0 so the counting starts over...
  }

  //get the key and request
  if (parseDataKey(resp).endsWith("mmxmnqtwnt")) {
    Serial.println("Key: ");
    Serial.println (parseDataKey(resp));
    
    Serial.println("Mode: ");
    Mode = parseDataAsString(resp, 1).toInt();
    Serial.println(Mode);
    
    if (Mode != preferences.getInt("Mode", 0)){
      preferences.putInt("Mode", Mode);
    }
    
    Serial.println("HumMax: ");
    HumMax = parseDataAsString(resp, 2).toInt();
    Serial.println(HumMax);
    
    if (HumMax != preferences.getInt("HumMax", 324)){
      preferences.putInt("HumMax", HumMax);
    }
    
    Serial.println("HumMin: ");
    HumMin = parseDataAsString(resp, 3).toInt();
    Serial.println(HumMin);

    if (HumMin != preferences.getInt("HumMin", 100)){
      preferences.putInt("HumMin", HumMin);
    }
    
    Serial.println("Quantity: ");
    Quantity = parseDataAsString(resp, 4).toInt();
    Serial.println(Quantity);

    if (Quantity != preferences.getInt("Quantity", 1500)){
      preferences.putInt("Quantity", Quantity);
    }
    
    Serial.println("WaterNow: ");
    WaterNow = parseDataAsString(resp, 5).toInt();
    Serial.println(WaterNow);
    
    
    Serial.println("Timer: ");
    Timer = parseDataAsString(resp, 6).toInt();
    Serial.println(Timer);

    if (Quantity != preferences.getInt("Timer", 12)){
      preferences.putInt("Timer", Timer);
    }
    
  }
  
  client.stop();

}





//Functions

//parse server response

String serverResponse (String httpresponse) {
  //Gets the httpresponse, removes files and keeps request
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
  int n = 0;
  
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid.c_str(), pass.c_str());
    // wait 10 seconds for connection:
    delay(10000);
    
    n = n + 1;
    Serial.println("Attempt: ");
    Serial.println(n);
    if (n > 3) {
    Serial.println("Connection Timeout");
    break;
    }
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
            //Build JSON response : {"DeviceID":"","WaterID":"","Mode":"","HumMax":"","HumMin":"","Quantity":"","Timer":"","dssid":"","ssid":"","WiFiConnected":"","extraData":""}
            String q = "&quot;";
            String c = "&comma;";

            String JSON = "{" + 
                              q + "DeviceID" + q + ":" + q + DeviceID + q + c +
                              q + "Mode" + q + ":" + q + String(Mode) + q + c +
                              q + "Humlvl" + q + ":" + q + String(humlvl) + q + c +
                              q + "HumMax" + q + ":" + q + String(HumMax) + q + c +
                              q + "HumMin" + q + ":" + q + String(HumMin) + q + c +
                              q + "Quantity" + q + ":" + q + String(Quantity) + q + c +
                              q + "Timer" + q + ":" + q + String(Timer) + q + c +
                              q + "dssid" + q + ":" + q + dssid + q + c +
                              q + "ssid" + q + ":" + q + ssid + q + c +
                              q + "WiFiConnected" + q + ":" + q + WiFiConnected + q + c +
                              q + "extraData" + q + ":" + q + extraData + q +
                           "}";
             Serial.println(JSON);
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            // the content of the HTTP response follows the header:
            client.print(JSON);
            client.print("<br>");

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

