
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
 
 
// CONSTANTS
 
const char* ssid = "YOUR_SSID";
const char* password = "MYSECRETPASS";
const int buttonPin = 2;     // the number of the pushbutton pin
const int ledPin =  13;      // the number of the LED pin
const char* HOST = "api.host.name";
const int PORT = 443;
const char* URL = "api/url";
const char* LOG_URL = "log.url";
bool occupied = false;
bool active = true;
 
String settingsLocation = "toilet-location";
int settingsPersonInBoothDistance = 100; //1m
const char* settingsGender = "m"; //m = male, f = female
String settingsId = "2";
 
// defines pins distance meter numbers
const int trigPin = 5;
const int echoPin = 4;
 
 
WiFiServer server(80);
 
// VARS
int buttonState = 0;         // variable for reading the pushbutton status
long duration;
int distance;
int boothBusy = false;
 
 
 
void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin, INPUT);
  // init US sensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
 
 
 
 
  Serial.begin(115200);
  delay(10);
 
   
  connectToWifi(ssid, password);
 
  Serial.println("Loading server...");
  server.begin();
 
  Serial.println("Sending log...");
  sendHttpsRequest(HOST, PORT, buildLogRequestStr(HOST, LOG_URL));
 
  Serial.println("Mac: " + WiFi.macAddress());
 
  Serial.println("IP: " + WiFi.localIP().toString());
 
}
 
int counter = 0;
void loop() {
 
  // read the state:
  delay(1000);
  int dist = getDistance();
  if (dist > 0 && dist < settingsPersonInBoothDistance) {
    if (!boothBusy) {
      boothBusy = true;
      digitalWrite(ledPin, HIGH);
       sendHttpsRequest(HOST, PORT, buildRequestStr(HOST, URL, true));
    }
  } else {
    if (boothBusy) {
      boothBusy = false;
      digitalWrite(ledPin, LOW);
      sendHttpsRequest(HOST, PORT, buildRequestStr(HOST, URL, false));
    }
  }
 
  handleServer();
 
 
}
 
 
 
 
 
 
//****************utility functions***********************
 
 
int getDistance() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  // Prints the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.println(distance);
  return distance;
}
 
 
String buildRequestStr(const char * _host, String _url, boolean _occupied) {
  String body_open = String("{\"id\": \"") + settingsId + "\",  \"location\": \"" + settingsLocation + "\", \"gender\":\"" + settingsGender + "\", \"status\": \"Open\"}";
  String body_closed = String("{\"id\": \"") + settingsId + "\",  \"location\": \"" + settingsLocation + "\", \"gender\":\"" + settingsGender + "\", \"status\": \"Closed\"}";
  String _body = (_occupied ? body_closed : body_open);
  String requestString = String("POST ") + _url + " HTTP/1.1\r\n" +
                         "Host: " + _host + "\r\n" +
                         "Content-Type: application/json\r\n" +
                         "Content-Length:" + _body.length()  + "\r\n" +
                         "Connection: close\r\n\r\n" +
                         _body;
  return requestString;
}
 
String buildLogRequestStr(const char * _host, String _url) {
  String mac = WiFi.macAddress();
  String ip = WiFi.localIP().toString();
  String _body = "{\"mac\":\""+ mac +"\" ,\"text\":\"IP: "+ ip +"\"}";
  String requestString = String("POST ") + _url + " HTTP/1.1\r\n" +
                         "Host: " + _host + "\r\n" +
                         "Content-Type: application/json\r\n" +
                         "Content-Length:" + _body.length()  + "\r\n" +
                         "Connection: close\r\n\r\n" +
                         _body;
  return requestString;
}
 
 
void sendHttpsRequest(const char * _host, int _port, String _requestStr) {
  Serial.print("connecting to ");
  Serial.println(_host);
  // Use WiFiClient class to create TCP connections
  WiFiClientSecure client;
  if (!client.connect(_host, _port)) {
    Serial.println("connection failed");
    return;
  }
 
  Serial.print("Requesting : \r\n\r\n");
  // Print to monitor - DEBUG
  Serial.print(_requestStr);
  // This will send the request to the server
  client.print(_requestStr);
 
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
 
  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
}
 
 
void connectToWifi(const char* _ssid, const char* _password) {
  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
 
 
void handleServer() {
  //Serial.println("handle server ...");
    // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
  // Match the request
  int value = LOW;
  if (request.indexOf("/LOCATION=") != -1)  {
    String tmp = request.substring(request.indexOf("/LOCATION=")+10);
    settingsLocation = tmp.substring(0, tmp.indexOf(" "));
    Serial.println(settingsLocation+"|L");
  }
  if (request.indexOf("/DISTANCE=") != -1)  {
    String tmp = request.substring(request.indexOf("/DISTANCE=")+10);
    tmp = tmp.substring(0,tmp.indexOf(" "));
    settingsPersonInBoothDistance = tmp.toInt();
    Serial.println(settingsPersonInBoothDistance+"|D");
  }
  if (request.indexOf("/ID=") != -1)  {
    String tmp = request.substring(request.indexOf("/ID=")+4);
    settingsId = tmp.substring(0, tmp.indexOf(" "));
    Serial.println(settingsId + +"|I");
  }
// Set ledPin according to the request
//digitalWrite(ledPin, value);
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
   int dist = getDistance();
 
  client.println("<br><br>");
  client.println("<a href=\"/LOCATION=TLV-26FL\"\"><button>Location (" + String(settingsLocation) + ")</button></a>");
  client.println("<a href=\"/ID=2\"\"><button>ID (" + String(settingsId) + ")</button></a>");
  client.println("<a href=\"/DISTANCE="+ String(dist) +"\"\"><button>Distance (" + settingsPersonInBoothDistance + ")</button></a><br />"); 
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
 
}
 
 
 
 
// DEBUG PRINT
//  Serial.print("active:");
//  Serial.print(active);
//  Serial.print("    Button State:");
//  Serial.print(buttonState);
//  Serial.print("    Occpied:");
//  Serial.print(occupied);
////  Serial.println("");
//
//      digitalWrite(ledPin, LOW);
//      Serial.println("FREE");
//      Serial.print("distanse: ");
//      Serial.println(dist);
//      Serial.print("busy: ");
//      Serial.println(boothBusy);
 
// Button sensor emulation
 
//  buttonState = digitalRead(buttonPin);
//
//  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
//  if (buttonState == HIGH && active) {
//      active = false;
//      occupied=!occupied;
//      digitalWrite(ledPin, (occupied?HIGH:LOW));
//
//      // Send POST
//      String buildRequestStrResult = buildRequestStr(HOST,URL, occupied);
//      sendHttpsRequest(HOST, PORT, buildRequestStrResult);
//  } else {
//    // turn LED off:
//    active = true;
//  }

