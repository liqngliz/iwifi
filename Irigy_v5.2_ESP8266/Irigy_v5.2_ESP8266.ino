#include <ESP8266WiFi.h>

String ssid = "Livebox-B15C";      //  your network SSID (name)
String pass = "C2F59CD5699AF134E5E9145E4F";   // your network password
String dssid = "irigyv1";      //  your network SSID (name)
String dpass = "irigyv12";   // your network password

const char* host = "http://oneviewdigital.com/";
const char* streamId   = "....................";
const char* privateKey = "....................";

bool wificonnected = false;

WiFiServer server(80);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(10);
  WiFi.mode(WIFI_STA);
  server.begin();
  
  WiFi.softAP(dssid.c_str(), dpass.c_str());
  
  
 
}


void loop() {

  if(WiFi.status() == WL_CONNECTED) {
  Serial.println("WiFi connected");
  }
  
  // put your main code here, to run repeatedly:
  WiFiClient clientAP = server.available();   // listen for incoming clients
  //Start AP server and print out all device data
  if (clientAP) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String Data = "";                       //Store data of GET request
    Data = readRequest(clientAP); //Parse the incoming request to get the GET request string
   
    // logon to Wifi
    Serial.println("Key: ");
    Serial.println (parseDataKey(Data));
    if (parseDataKey(Data).endsWith("pwssid") && wificonnected == false){
      
      Serial.println("new SSID: ");
      Serial.println (parseDataAsString(Data, 1));
      ssid = parseDataAsString(Data, 1);
      
      Serial.println("new Pass: ");
      Serial.println (parseDataAsString(Data, 2));
      pass = parseDataAsString(Data, 2);

      //Wifi connection
      wifiConnection();
    }

     // close the connection:
    clientAP.stop();
    Serial.println("client disonnected");
  }

  delay(5000);

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/?DeviceID=18c29d17-71ea-11e7-9c8c-e0db55fe78c8&humlvl=280&Alert=0&WateredID=18c29d31-71ea-11e7-9c8c-e0db55fe78c8";


  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");
  

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



void wifiConnection() {

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  wificonnected = true;

}

