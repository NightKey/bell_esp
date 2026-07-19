#include <Arduino.h>
#include <Log.h>
#include <cmath>


class SensorData {
    public:
        float pressure;
        float temperature;
        float humidity;
        float heatIndex;
        bool isFahrenheit;

        SensorData() {
            constexpr auto temp = static_cast<float>(-99);
            pressure = temp;
            cTemperature = temp;
            fTemperature = convertToFahrenheit(temp);
            humidity = temp;
            isFahrenheit = false;
            temperature = isFahrenheit ? fTemperature : cTemperature;
            heatIndex = calculateHeatIndex();
        }

        SensorData(const float _pressure, const float _temperature, const float _humidity, const bool _isFahrenheit) {
            pressure = _pressure;
            cTemperature = _temperature;
            fTemperature = convertToFahrenheit(_temperature);
            humidity = _humidity;
            isFahrenheit = _isFahrenheit;
            temperature = isFahrenheit ? fTemperature : cTemperature;
            heatIndex = calculateHeatIndex();
        }

        String toString() {
            timer.startNewTimer("ToString");
            String tmpUnit = "°C";
            if (isFahrenheit) tmpUnit = "°F";
            String data = R"({"temperature":")" + String(temperature) + R"(", "heat_index":")" + String(heatIndex) + R"(","temperature_unit":")" + tmpUnit + R"(","humidity":")" + String(humidity) + R"(","pressure":")" + String(pressure) + "\"}";
            timer.stopAndLog("ToString");
            return data;
        }

    private:
        float cTemperature;
        float fTemperature;
        Timer timer = Timer();
        static float convertToFahrenheit(const float temp) {
            return static_cast<float>(9.0/5.0) * temp + 32;
        }

        static float convertToCelsius(const float temp) {
            return static_cast<float>(5.0/9.0) * (temp - 32);
        }

        float calculateHeatIndex() {
            timer.startNewTimer("HeatIndex calculation");
            const float tempSq = fTemperature*fTemperature;
            const float humSq = humidity*humidity;
            auto tmp = static_cast<float>(-42.379 + (2.04901523*fTemperature) + (10.14333127*humidity) - (0.22475541*fTemperature*humidity) - (0.00683783*tempSq) - (0.05481717*humSq) + (0.00122874*tempSq*humidity) + (0.00085282*fTemperature*humSq) - (0.00000199*tempSq*humSq));
            debugln("tempSq: " + String(tempSq));
            debugln("humSq: " + String(humSq));
            debugln("TMP: " + String(tmp));

            if (humidity < 13 && fTemperature >= 80 && fTemperature <= 110) {
                tmp -= static_cast<float>(((13 - humidity )/4)  * sqrt(17 - abs(fTemperature - 95.0) / 17));
            } else if (humidity >85 && fTemperature > 80 && fTemperature < 87) {
                tmp += ((humidity -85) / 10) * ((87 - fTemperature) / 5);
            } else if (fTemperature < 80){
                tmp = static_cast<float>(0.5 * (fTemperature  + 61.0 + ((fTemperature - 68.0) * 1.2) + (humidity * 0.094)));
            }

            debugln("Adjusted: " + String(tmp));

            timer.stopAndLog("HeatIndex calculation");
            return isFahrenheit ? tmp : convertToCelsius(tmp);
        }
};
