#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include <Arduino.h>

struct PlantConfig {
    String name;
    float minMoisture;
    float maxMoisture;

    float dryVoltage;
    float wetVoltage;
};

class SoilSensor {
public:
    SoilSensor(uint8_t pin);

    void update();

    PlantConfig getPlantConfig() const;
    float getVoltage() const;
    float getPercent() const;
    String getStatus() const;
    void setPlantConfig(const PlantConfig& config);

private:
    uint8_t _pin;
    float _voltage;
    float _percent;
    PlantConfig _plant;
};

#endif