#include <Arduino.h>
#include <map>

class Timer {
    public:
        Timer()= default;
        void startNewTimer(const String& name) {
            #if DEBUG >= 1
            timers[name] = millis();
            #endif
        }
        void stopAndLog(const String& name) {
            #if DEBUG >= 1
            const auto _timer = timers.find(name);
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
            #if DEBUG >= 2
            debugln(name + " took " + getDurationString(duration));
            #endif
            timers.erase(_timer);
            yield();
            #endif
        }
    private:
        #if DEBUG >= 1
        std::map<String, unsigned long> timers;
        static String getDurationString(const unsigned long duration) {
            if(duration / 1000 > 0) {
                const float seconds = duration / 1000.0;
                return String(seconds) + " s";
            }
            return String(duration) + " ms";
        }
        #endif
};