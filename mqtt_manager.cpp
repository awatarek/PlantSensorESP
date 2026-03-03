#include "mqtt_manager.h"
#include <WiFi.h>

MQTTManager::MQTTManager(AsyncWebServer& server,
                         AsyncMqttClient& mqttClient,
                         Preferences& prefs)
    : _server(server),
      _mqttClient(mqttClient),
      _prefs(prefs) {}

void MQTTManager::loadConfig() {
    _prefs.begin("config", true);
    _broker = _prefs.getString("broker", "");
    _user   = _prefs.getString("muser", "");
    _pass   = _prefs.getString("mpass", "");
    _prefs.end();
}

void MQTTManager::begin() {

    loadConfig();

    if (_broker == "") {
        return;
    }

    _mqttClient.setServer(_broker.c_str(), 1883);
    _mqttClient.setCredentials(_user.c_str(), _pass.c_str());

    _mqttClient.onConnect([this](bool sessionPresent) {
        Serial.println("MQTT Connected!");
        publishDiscovery();
    });

    _mqttClient.onDisconnect([](AsyncMqttClientDisconnectReason reason) {
        Serial.print("MQTT Disconnected. Reason: ");
        Serial.println((int)reason);
    });

    _mqttClient.connect();
}

String MQTTManager::getMacAddress() {

    uint64_t chipid = ESP.getEfuseMac();

    char macStr[13];
    sprintf(macStr, "%04X%08X",
            (uint16_t)(chipid >> 32),
            (uint32_t)chipid);

    return String(macStr);
}

void MQTTManager::publishDiscovery() {
    publishSensor("soil", "Soil Moisture", "%", "humidity");
    publishSensor("voltage", "Soil Voltage", "V", "");
    publishSensor("status", "Soil Status", "", "");
}

void MQTTManager::publishSensor(const String& id,
                                const String& name,
                                const String& unit,
                                const String& deviceClass) {

    String mac = getMacAddress();

    String baseTopic = "plant/" + mac;
    String stateTopic = baseTopic + "/" + id;

    String configTopic =
        "homeassistant/sensor/plant_sensor_" + mac + "_" + id + "/config";

    String uniqueId = "plant_sensor_" + mac + "_" + id;

    String payload = "{";
    payload += "\"name\":\"" + name + "\",";
    payload += "\"state_topic\":\"" + stateTopic + "\",";

    if (unit != "")
        payload += "\"unit_of_measurement\":\"" + unit + "\",";

    if (deviceClass != "")
        payload += "\"device_class\":\"" + deviceClass + "\",";

    payload += "\"unique_id\":\"" + uniqueId + "\",";

    payload += "\"device\":{";
    payload += "\"identifiers\":[\"plant_sensor_" + mac + "\"],";
    payload += "\"name\":\"Plant Sensor " + mac + "\",";
    payload += "\"manufacturer\":\"DIY\",";
    payload += "\"model\":\"ESP32 Plant Monitor\"";
    payload += "}}";

    _mqttClient.publish(configTopic.c_str(), 0, true, payload.c_str());
}

void MQTTManager::saveCredentials(const String& broker,
                                  const String& user,
                                  const String& pass) {

    _prefs.begin("config", false);
    _prefs.putString("broker", broker);
    _prefs.putString("muser", user);
    _prefs.putString("mpass", pass);
    _prefs.end();

    _broker = broker;
    _user   = user;
    _pass   = pass;
}

void MQTTManager::clearCredentials() {

    _prefs.begin("config", false);
    _prefs.remove("broker");
    _prefs.remove("muser");
    _prefs.remove("mpass");
    _prefs.end();

    _broker = "";
    _user   = "";
    _pass   = "";
}

void MQTTManager::publishValue(const String& id, const String& value) {

    if (!_mqttClient.connected()) return;

    String mac = getMacAddress();
    String topic = "plant/" + mac + "/" + id;

    _mqttClient.publish(topic.c_str(), 0, false, value.c_str());
}

bool MQTTManager::isConfigured() {
    return _broker != "";
}

bool MQTTManager::isConnected() {
    return _mqttClient.connected();
}