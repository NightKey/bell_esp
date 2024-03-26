#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>

#include <Data.cpp>
#include <Config.h>
#include <WebServer.cpp>
#include <Log.h>

#define SEALEVELPRESSURE_HPA (1013.25)

#define RINGSWITCH 13

Adafruit_BME280 bme;
bool status;
WebServer server;

bool useFahrenheit = false;

void gatherValues();
void bellRang();
void toggleFahrenhei();

void setup() {
  start(9600);
  // Initializing BME280
  status = bme.begin(0x76);
  if (!status) {
    debugln("Couldn't find BME280!");
    while(1);
  }
  // Connecting to WIFI
  WiFi.mode(WIFI_STA);
  debugln("Connecting to WIFI according to config...");
  WiFi.begin(WiFiSettings.ssid, WiFiSettings.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    if (WiFi.status() == WL_CONNECT_FAILED) {
      debugln("WiFi connection failed!");
      while(1);
    }
  }
  // Creating webserver
  server = WebServer(WiFiSettings.port);
  // Initializing pins
  pinMode(RINGSWITCH, INPUT);
  attachInterrupt(RINGSWITCH, bellRang, FALLING);
}

void loop() {
  server.loop();
}

void WebServer::commandRetrived(String target) {
  if (target == "toggleFahrenheit") toggleFahrenhei();
  else if (target == "getSensors") gatherValues();
  else {
    send("Not a valid command");
  }
}

void bellRang() {
  server.send("Bell");
}

void toggleFahrenhei() {
  useFahrenheit = !useFahrenheit;
  server.send(String(useFahrenheit));
}

void gatherValues() {
  float temp = bme.readTemperature();
  if (useFahrenheit) {
    temp = 1.8 * temp + 32;
  }
  debugln("Current Temp.: " + String(temp));
  float hum = bme.readHumidity();
  debugln("Current Hum.: " + String(hum));
  float pres = bme.readPressure();
  debugln("Current Pres.: " + String(pres));
  SensorData data = SensorData(pres, temp, hum, useFahrenheit);
  server.send(data.toString());
}
