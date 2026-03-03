#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include <Arduino.h>

class SoilSensor {
public:
    SoilSensor(uint8_t pin);

    void update();

    float getVoltage() const;
    float getPercent() const;
    String getStatus() const;

private:
    uint8_t _pin;
    float _voltage;
    float _percent;
};

#endif