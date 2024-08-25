#include <Arduino.h>
#include <Log.h>
#include <math.h>


class SensorData {
    public:
        float pressure;
        float temperature;
        float humidity;
        float heatIndex;
        bool isFahrenheit;

        SensorData() {
            SensorData(-99, -99, -99, 0);
        }

        SensorData(float _pressure, float _temperature, float _humidity, bool _isFahrenheit) {
            pressure = _pressure;
            cTemperature = _temperature;
            fTemperature = convertToFahrenheit(_temperature);
            humidity = _humidity;
            isFahrenheit = _isFahrenheit;
            temperature = isFahrenheit ? fTemperature : cTemperature;
            calculateHeatIndex();
        }

        String toString() {
            timer.startNewTimer("ToString");
            String tmpUnit = "°C";
            if (isFahrenheit) tmpUnit = "°F";
            String data = "{\"temperature\":\"" + String(temperature) + "\", \"heat_index\":\"" + String(heatIndex) + "\",\"temperature_unit\":\"" + tmpUnit + "\",\"humidity\":\"" + String(humidity) + "\",\"pressure\":\"" + String(pressure) + "\"}";
            timer.stopAndLog("ToString");
            return data;
        }

    private:
        float cTemperature;
        float fTemperature;
        Timer timer = Timer();
        float convertToFahrenheit(float temp) { 
            return (9.0/5.0) * temp + 32;
        }

        float convertToCelsius(float temp) {
            return (5.0/9.0) * (temp - 32);
        }

        void calculateHeatIndex() {
            timer.startNewTimer("HeatIndex calculation");
            float tempSq = fTemperature*fTemperature;
            float humSq = humidity*humidity;
            float tmp = -42.379 + (2.04901523*fTemperature) + (10.14333127*humidity) - (0.22475541*fTemperature*humidity) - (0.00683783*tempSq) - (0.05481717*humSq) + (0.00122874*tempSq*humidity) + (0.00085282*fTemperature*humSq) - (0.00000199*tempSq*humSq);
            debugln("tempSq: " + String(tempSq));
            debugln("humSq: " + String(humSq));
            debugln("TMP: " + String(tmp));

            if (humidity < 13 && fTemperature >= 80 && fTemperature <= 110) {
                tmp -= ((13 - humidity )/4)  * sqrt(17 - abs(fTemperature - 95.0) / 17);
            } else if (humidity >85 && fTemperature > 80 && fTemperature < 87) {
                tmp += ((humidity -85) / 10) * ((87 - fTemperature) / 5);
            } else if (fTemperature < 80){
                tmp = 0.5 * (fTemperature  + 61.0 + ((fTemperature - 68.0) * 1.2) + (humidity * 0.094));
            }

            debugln("Adjusted: " + String(tmp));

            heatIndex = isFahrenheit ? tmp : convertToCelsius(tmp);
            timer.stopAndLog("HeatIndex calculation");
        }
};
