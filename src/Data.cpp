#include <Arduino.h>


class SensorData {
    public:
        float pressure;
        float temperature;
        float humidity;
        float heatIndex;

        SensorData(float _pressure, float _temperature, float _humidity, bool _isFahrenheit) {
            pressure = _pressure;
            temperature = _temperature;
            humidity = _humidity;
            isFahrenheit = _isFahrenheit;
            calculateHeatIndex();
        }

        String toString() {
            String tmpUnit = "°C";
            if (isFahrenheit) tmpUnit = "°F";
            return "{\"temperature\":\"" + String(temperature) + "\", \"heat_index\":\"" + String(heatIndex) + "\",\"temperature_unit\":\"" + tmpUnit + "\",\"humidity\":\"" + String(humidity) + "\",\"pressure\":\"" + String(pressure) + "\"}";
        }

    private:
        bool isFahrenheit;
        void calculateHeatIndex() {
            float c1,c2,c3,c4,c5,c6,c7,c8,c9;
            float tempSq = temperature*temperature;
            float humSq = humidity*humidity;
            if (isFahrenheit) {
                c1 = -42.37;
                c2 = 2.04901523;
                c3 = 10.14333127;
                c4 = -0.22475541;
                c5 = -0.00683783;
                c6 = -0.05481717;
                c7 = 0.0012874;
                c8 = 0.00085282;
                c9 = -0.00000199;
            } else {
                c1 = -8.784694755556;
                c2 = 1.61139411;
                c3 = 2.33854883889;
                c4 = -0.14611605;
                c5 = -0.012308094;
                c6 = -0.0164248277778;
                c7 = 0,002211732;
                c8 = 0.00072546;
                c9 = -0.000003582;
            }
            heatIndex = c1 +  c2*temperature + c3*humidity + c4*temperature*humidity + c5*tempSq + c6*humSq + c7*tempSq*humidity + c8*temperature*humSq + c9*tempSq*humSq;
        }
};
