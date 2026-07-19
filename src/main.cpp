#include <Log.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>

#include <Data.cpp>
#include <Config.h>
#include <WebServer.cpp>

#define SEALEVELPRESSURE_HPA (1013.25)

#define BELLSWITCH 13

#if DEBUG >= 3
SensorData simulation(1000000.0, -20.0, 50.0, false);
#endif

enum Health {
  WIFI,
  SERVER,
  BME280,
  HEALTHY,
};

static Adafruit_BME280 bme;
static bool status;
static WebServer server(WiFiSettings.port, WiFiSettings.maxClients);
static unsigned long debounceTimer;
static unsigned long blinkTimer;
static bool blinkState;
static int blinkStep = 0;
static bool bellDetected = false;
static Health health = Health::HEALTHY;

static bool useFahrenheit = false;

static SensorData gatherValues();
static void bellRang();
static String toggleFahrenheit();
static void healthCheck();
static Timer timer;
static Timer gatherTimer;

void setup() {
  start(9600);
  debugln("Bell ESP Setup started");
  pinMode(BELLSWITCH, INPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  timer = Timer();
  // Initializing BME280
  timer.startNewTimer("Total setup");
  timer.startNewTimer("BME280 setup");
  status = bme.begin(0x76);
  if (!status) {
    debugln("Couldn't find BME280!");
    health = Health::BME280;
  }
  timer.stopAndLog("BME280 setup");
  // Connecting to WIFI
  timer.startNewTimer("WiFi setup");
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);
  Serial.printf("WiFi Station MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

  WiFiClass::mode(WIFI_STA);
  WiFi.config(WiFiSettings.local_IP, WiFiSettings.gateway, WiFiSettings.subnet);
  debug("Connecting to \"" + String(WiFiSettings.ssid) + "\" WIFI");
  WiFi.begin(WiFiSettings.ssid, WiFiSettings.password);
  while (WiFiClass::status() != WL_CONNECTED) {
    delay(500);
    debug(".");
    if (WiFiClass::status() == WL_CONNECT_FAILED) {
      debugln("\nWiFi connection failed!");
      if (health == Health::HEALTHY) health = Health::WIFI;
    }
  }
  timer.stopAndLog("WiFi setup");
  debugln("");
  // Creating webserver
  timer.startNewTimer("Server setup");
  if (health == Health::HEALTHY) {
    if (server.begin()) {
      debugln("IP Address: " + String(WiFi.localIP().toString()) + ":" + String(WiFiSettings.port));
    } else {
      health = Health::SERVER;
    }
  }
  timer.stopAndLog("Server setup");
  // Initializing pins
  timer.stopAndLog("Total setup");
  #if DEBUG >= 1
  gatherValues();
  #endif
}

static void WIFIBlink() {
  switch (blinkStep) {
    case 0:
      blinkState = true;
      blinkTimer = millis();
      ++blinkStep;
      break;
    case 1:
      if (millis() - blinkTimer >= 500) {
        blinkState = false;
        blinkTimer = millis();
        ++blinkStep;
      }
      break;
    case 2:
      if (millis() - blinkTimer >= 1000) {
        blinkStep = 0;
      }
      break;
    default:
      blinkStep = 0;
      break;
  }
}

static void ServerBlink() {
  switch (blinkStep) {
    case 0:
      blinkState = true;
      blinkTimer = millis();
      ++blinkStep;
      break;
    case 1:
      if (millis() - blinkTimer >= 500) {
        blinkState = false;
        blinkTimer = millis();
        ++blinkStep;
      }
      break;
    case 2:
      if (millis() - blinkTimer >= 500) {
        blinkState = true;
        blinkTimer = millis();
        ++blinkStep;
      }
    case 3:
      if (millis() - blinkTimer >= 500) {
        blinkState = false;
        blinkTimer = millis();
        ++blinkStep;
      }
      break;
    case 4:
      if (millis() - blinkTimer >= 1000) {
        blinkStep = 0;
      }
      break;
    default:
      blinkStep = 0;
      break;
  }
}

static void BMEBlink() {
  switch (blinkStep) {
    case 0:
      blinkState = true;
      blinkTimer = millis();
      ++blinkStep;
      break;
    case 1:
      if (millis() - blinkTimer >= 500) {
        blinkState = false;
        blinkTimer = millis();
        ++blinkStep;
      }
      break;
    case 2:
      if (millis() - blinkTimer >= 500) {
        blinkState = true;
        blinkTimer = millis();
        ++blinkStep;
      }
      break;
    case 3:
      if (millis() - blinkTimer >= 500) {
        blinkState = false;
        blinkTimer = millis();
        ++blinkStep;
      }
      break;
    case 4:
      if (millis() - blinkTimer >= 500) {
        blinkState = true;
        blinkTimer = millis();
        ++blinkStep;
      }
      break;
    case 5:
      if (millis() - blinkTimer >= 500) {
        blinkState = false;
        blinkTimer = millis();
        ++blinkStep;
      }
      break;
    case 6:
      if (millis() - blinkTimer >= 1000) {
        blinkStep = 0;
      }
      break;
    default:
      blinkStep = 0;
      break;
  }
}

static void blinkLed() {
  switch (health) {
    case WIFI:
      WIFIBlink();
      break;
    case SERVER:
      ServerBlink();
      break;
    case BME280:
      BMEBlink();
      break;
    case HEALTHY:
      if (bellDetected) {
        blinkState = true;
        if (millis() - blinkTimer >= 1000) {
          bellDetected = false;
          blinkTimer = millis();
        }
      } else {
        blinkState = false;
        blinkStep = 0;
        blinkTimer = millis();
      }
      #if DEBUG >= 1
      if (millis() - blinkTimer > 1000) {
        debug(".");
        blinkTimer = millis();
      }
      #endif
      break;
  }
  digitalWrite(BUILTIN_LED, blinkState);
}

void loop() {
  timer.startNewTimer("Main loop");
  if(!digitalRead(BELLSWITCH)) {
    if (millis() - debounceTimer <= 5000) {
      #if DEBUG > 2
      debugln("Debounding....")
      #endif
      debounceTimer = millis();
    } else {
      debounceTimer = millis();
      bellRang();
    }
  }
  if (health != Health::HEALTHY) {
    blinkLed(); // Do not do anything when not healthy
    return;
    timer.stopAndLog("Main loop");
  }
  healthCheck();
  blinkLed();
  if (server.loop()) {
    debugln("Web Server failed");
  }
  timer.stopAndLog("Main loop");
}

static bool I2CHealth(const int address) {
  Wire.beginTransmission(address);
  const byte error = Wire.endTransmission(true);
  return error == 0;
}

void healthCheck() {
  health = Health::HEALTHY;
  if (WiFiClass::status() != WL_CONNECTED) {
    health = Health::WIFI;
  }
  if (!server.isHealthy()) {
    health = Health::SERVER;
  }
  if (!I2CHealth(0x76) || bme.sensorID() != 0x60 || isnanf(bme.readTemperature()) || isnanf(bme.readHumidity()) || isnanf(bme.readPressure())) {
    health = Health::BME280;
  }
}

void WebServer::commandRetrieved(WiFiClient &sender, const String &command) {
  timer.startNewTimer("Retrieving command");
  if (command == "toggleFahrenheit") send(sender, toggleFahrenheit());
  else if (command == "getSensors") send(sender, gatherValues().toString());
  else if (command == "ping") {
    send(sender, "pong");
  }
  else {
    send(sender, "Not a valid command");
  }
  timer.stopAndLog("Retrieving command");
}

void bellRang() {
  debugln("BellRangCalled");
  server.sendAll("Bell");
  bellDetected = true;
}

String toggleFahrenheit() {
  useFahrenheit = !useFahrenheit;
  return String(useFahrenheit);
}

static void logSensorData(const SensorData& data) {
  const String tempUnit = data.isFahrenheit ? " °F" : " °C";
  debugln("Current Temp.: " + String(data.temperature) + tempUnit);
  debugln("Current Hum.: " + String(data.humidity) + " %");
  debugln("Current Pres.: " + String(data.pressure) + " pa");
  debugln("Current Perceived Temp.: " + String(data.heatIndex) + tempUnit);
}

SensorData gatherValues() {
  gatherTimer.startNewTimer("Sensor data gathering");
  auto data = SensorData();
  #if DEBUG >= 3
  if (simulation.temperature++ >= 40) {
    simulation.temperature = -20.0;
  }
  data = simulation;
  #else
  if (!status) {
    data = SensorData(-99, -99, -99, false);
  } else {
    const float temp = bme.readTemperature();
    const float hum = bme.readHumidity();
    const float pres = bme.readPressure();
    data = SensorData(pres, temp, hum, useFahrenheit);
  }
  #endif
  #if DEBUG >= 2
  logSensorData(data);
  #endif
  gatherTimer.stopAndLog("Sensor data gathering");
  return data;
}
