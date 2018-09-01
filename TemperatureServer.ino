#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ArduinoJson.h>
#include <FS.h>

#define ONE_WIRE_BUS 14

const char* ssid;
const char* password;
const char* hostname;


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

ESP8266WebServer server(80);


void setup() {
  Serial.begin(9600);
  SPIFFS.begin();

  loadConfig();
  setupWiFi();
  setupWebServer();

  sensors.begin();
}

void loop() {
  server.handleClient();
}

boolean loadConfig() {
  File configFile = SPIFFS.open("/config.json", "r");
  if(!configFile) {
    Serial.println("Failed to open config file");
  }

  size_t size = configFile.size();
  if(size > 1024) {
    Serial.println("Config file is too large");
    return false;
  }

  std::unique_ptr<char[]> buf(new char[size]);
  configFile.readBytes(buf.get(), size);

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(buf.get());

  if(!json.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  ssid = json["ssid"];
  password = json["password"];
  hostname = json["hostname"];

  return true;
}

void setupWiFi() {
  Serial.println("Connecting to wifi...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to: ");
  Serial.println(ssid);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  if(MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
  }
}

void setupWebServer() {
  server.on("/", handleRoot);
  server.on("/fahrenheit", handleFahrenheit);
  server.on("/celsius", handleCelsius);
  server.on("/app.js", handleScript);
  
  server.onNotFound([] {
  });

  server.begin();
}

void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleScript(){
  File file = SPIFFS.open("/app.js", "r");
  server.streamFile(file, "application/javascript");
  file.close();
}

void handleFahrenheit() {
  char buffer[23];
  
  sensors.requestTemperatures();
  float temp = sensors.getTempFByIndex(0);

  sprintf(buffer, "{\"temperature\": %3.2f}", temp);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", buffer);
}

void handleCelsius(){
  char buffer[23];

  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);

  sprintf(buffer, "{\"temperature\": %3.2f}", temp);

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", buffer);
}
