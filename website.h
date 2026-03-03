#ifndef WEBSITE_H
#define WEBSITE_H

#include <Arduino.h>
#include <AsyncMqttClient.h>

class SoilSensor;

class Website {
public:
    Website(AsyncMqttClient& mqttClient);

    String build(const SoilSensor& soil,
                bool wifiConnected,
                bool mqttConfigured,
                bool mqttConnected,
                const String& networkList);

private:
    AsyncMqttClient& _mqttClient;
};

#endif