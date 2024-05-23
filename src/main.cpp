#define DEBUG 2
#include <Log.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>

#include <Data.cpp>
#include <Config.h>
#include <WebServer.cpp>

#define SEALEVELPRESSURE_HPA (1013.25)

#define RINGSWITCH 13

#if DEBUG >= 3
SensorData simulation(1000000.0, -20.0, 50.0, false);
#endif

Adafruit_BME280 bme;
bool status;
WebServer server(WiFiSettings.port);
int debounceTimer;

bool useFahrenheit = false;

SensorData gatherValues();
void bellRang();
void toggleFahrenhei();

void setup() {
  start(9600);
  // Initializing BME280
  status = bme.begin(0x76);
  if (!status) {
    debugln("Couldn't find BME280!");
    while(DEBUG < 2);
  }
  // Connecting to WIFI
  WiFi.mode(WIFI_STA);
  WiFi.config(WiFiSettings.local_IP, WiFiSettings.gateway, WiFiSettings.subnet);
  debug("Connecting to \"" + String(WiFiSettings.ssid) + "\" WIFI");
  WiFi.begin(WiFiSettings.ssid, WiFiSettings.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    debug(".");
    if (WiFi.status() == WL_CONNECT_FAILED) {
      debugln("\nWiFi connection failed!");
      while(1);
    }
  }
  debugln("");
  // Creating webserver
  server.begin();
  debugln("IP Address: " + String(WiFi.localIP().toString()) + ":" + String(WiFiSettings.port));
  // Initializing pins
  pinMode(RINGSWITCH, INPUT);
  #if DEBUG >= 1
  gatherValues();
  #endif
}

void loop() {
  server.loop();
  if(!digitalRead(RINGSWITCH)) {
    if (millis() - debounceTimer <= 1000) {
      debugln("Debounding....");
      debounceTimer = millis();
      return;
    }
    debounceTimer = millis();
    bellRang();
  }
}

void WebServer::commandRetrived(String target) {
  if (target == "toggleFahrenheit") toggleFahrenhei();
  else if (target == "getSensors") send(gatherValues().toString());
  else {
    send("Not a valid command");
  }
}

void bellRang() {
  debugln("BellRangCalled");
  server.send("Bell");
}

void toggleFahrenhei() {
  useFahrenheit = !useFahrenheit;
  server.send(String(useFahrenheit));
}

void logSensorData(SensorData data) {
  String tempUnit = data.isFahrenheit ? " °F" : " °C";
  debugln("Current Temp.: " + String(data.temperature) + tempUnit);
  debugln("Current Hum.: " + String(data.humidity) + " %");
  debugln("Current Pres.: " + String(data.pressure) + " pa");
  debugln("Current Perceived Temp.: " + String(data.heatIndex) + tempUnit);
}

SensorData gatherValues() {
  SensorData data = SensorData();
  #if DEBUG >= 3
  if (simulation.temperature++ >= 40) {
    simulation.temperature = -20.0;
  }
  data = simulation;
  #else
  if (!status) {
    data = SensorData(-99, -99, -99, false);
  } else {
    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float pres = bme.readPressure();
    data = SensorData(pres, temp, hum, useFahrenheit);
  }
  #endif
  logSensorData(data);
  return data;
}
