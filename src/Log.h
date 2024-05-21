#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG >= 2
#define CORE_DEBUG_LEVEL 5
#define CONFIG_ARDUHAL_LOG_COLORS 1
#endif
#if DEBUG >= 1
#include <Arduino.h>
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#define start(x) Serial.begin(x)
#else
#define debug(x)
#define debugln(x)
#define start(x)
#endif
