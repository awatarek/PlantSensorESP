#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <AsyncMqttClient.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

class MQTTManager {
public:
    MQTTManager(AsyncWebServer& server,
                AsyncMqttClient& mqttClient,
                Preferences& prefs);

    void registerSetupPage();
    void begin();
    void publishSoil(float percent);

private:
    AsyncWebServer& _server;
    AsyncMqttClient& _mqttClient;
    Preferences& _prefs;

    String _broker;
    String _user;
    String _pass;

    void loadConfig();
    void publishDiscovery();
    String getMacAddress();
};

#endif