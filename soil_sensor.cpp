#include "soil_sensor.h"

SoilSensor::SoilSensor(uint8_t pin)
    : _pin(pin), _voltage(0), _percent(0) {}

void SoilSensor::update() {

    _voltage = analogRead(_pin) * (3.3 / 4095.0);

    _percent = (2.2 - _voltage) * (100.0 / (2.2 - 0.89));
    _percent = constrain(_percent, 0, 100);
}

float SoilSensor::getVoltage() const {
    return _voltage;
}

float SoilSensor::getPercent() const {
    return _percent;
}

String SoilSensor::getStatus() const {

    if (_percent < _plant.minMoisture)
        return "Dry 🌵";

    else if (_percent < _plant.minMoisture + 5)
        return "Getting dry ⚠";

    else if (_percent > _plant.maxMoisture)
        return "Wet 💧";

    else if (_percent > _plant.maxMoisture - 5)
        return "Getting wet ⚠";

    else
        return "Optimal 🌿";
}
void SoilSensor::setPlantConfig(const PlantConfig& config) {
    _plant = config;
}

PlantConfig SoilSensor::getPlantConfig() const {
    return _plant;
}