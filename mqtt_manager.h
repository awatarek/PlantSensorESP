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

    void begin();
    void publishValue(const String& id, const String& value);
    bool isConfigured();
    bool isConnected();
    void saveCredentials(const String& broker,
                     const String& user,
                     const String& pass);
    void clearCredentials();
    
private:
    AsyncWebServer& _server;
    AsyncMqttClient& _mqttClient;
    Preferences& _prefs;

    String _broker;
    String _user;
    String _pass;

    void loadConfig();
    void publishDiscovery();    
    void publishSensor(const String& id,
                   const String& name,
                   const String& unit,
                   const String& deviceClass);
    String getMacAddress();
};

#endif