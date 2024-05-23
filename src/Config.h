#include <Arduino.h>

struct {
    const char* ssid = "SSID HERE";
    const char* password = "PASSWORD";
    const int port = 6900;
    const IPAddress local_IP = IPAddress(192, 168, 0, 20);
    const IPAddress gateway = IPAddress(192, 168, 0, 1);
    const IPAddress subnet = IPAddress(255, 255, 255, 0);
} WiFiSettings;
