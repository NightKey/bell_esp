#include <Arduino.h>

class Timer {
    public:
        #if DEBUG >=1
        unsigned long startTime;
        unsigned long duration;
        String name;
        #endif
        Timer(){}
        void startNewTimer(String newName) {
            #if DEBUG >= 1
            name = newName;
            startTime = millis();
            duration = 0;
            #else
            return;
            #endif
        }
        void stopAndLog() {
            #if DEBUG >= 1
            duration = millis() - startTime;
            if (duration <= 2) return;
            debugln(name + " took " + getDurationString());
            #else
            return;
            #endif
        }
    private:
        String getDurationString() {
            #if DEBUG >= 1
            if(duration / 1000 > 0) {
                float seconds = duration / 1000.0;
                return String(seconds) + " s";
            } else {
                return String(duration) + " ms";
            }
            #else
            return String("NaN");
            #endif
        }
};