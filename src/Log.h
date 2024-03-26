#include <Arduino.h>

#define DEBUG 1

#if DEBUG == 1
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define start(x) Serial.begin(x)
#else
#define debug(x)
#define debugln(x)
#define start(x)
#endif
