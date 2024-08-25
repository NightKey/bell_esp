#include <Arduino.h>
#include <map>

class Timer {
    public:
        Timer(){}
        void startNewTimer(String name) {
            #if DEBUG >= 1
            timers[name] = millis();
            #else
            return;
            #endif
        }
        void stopAndLog(String name) {
            #if DEBUG >= 1
            auto _timer = timers.find(name);
            if (_timer == timers.end()) {
                debugln("'" + name + "' was not present in the 'timers' map:");
                debugln("{");
                for(const auto& element : timers) {
                    debugln("\t'" + String(element.first) + "' : " + String(element.second) + ",");
                }
                debugln("}");
                return;
            }
            unsigned long duration = millis() - _timer->second;
            debugln(name + " took " + getDurationString(duration));
            timers.erase(_timer);
            #else
            return;
            #endif
        }
    private:
        #if DEBUG >= 1
        std::map<String, unsigned long> timers;
        String getDurationString(unsigned long duration) {
            if(duration / 1000 > 0) {
                float seconds = duration / 1000.0;
                return String(seconds) + " s";
            } else {
                return String(duration) + " ms";
            }
        }
        #endif
};