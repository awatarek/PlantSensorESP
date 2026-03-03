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

void MQTTManager::registerSetupPage() {

    _server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html",
            "<h2>MQTT Setup</h2>"
            "<form method='POST' action='/save_mqtt'>"
            "Broker IP:<br><input name='broker'><br>"
            "Username:<br><input name='user'><br>"
            "Password:<br><input name='pass' type='password'><br><br>"
            "<input type='submit' value='Save'>"
            "</form>");
    });

    _server.on("/save_mqtt", HTTP_POST, [this](AsyncWebServerRequest *request) {

        String broker = request->getParam("broker", true)->value();
        String user   = request->getParam("user", true)->value();
        String pass   = request->getParam("pass", true)->value();

        _prefs.begin("config", false);
        _prefs.putString("broker", broker);
        _prefs.putString("muser", user);
        _prefs.putString("mpass", pass);
        _prefs.end();

        request->send(200, "text/html",
            "<h3>MQTT Saved. Rebooting...</h3>");

        delay(1000);
        ESP.restart();
    });
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

    String mac = getMacAddress();

    String topic =
        "homeassistant/sensor/plant_sensor_" + mac + "/config";

    String uniqueId =
        "plant_sensor_" + mac + "_soil";

    String payload = "{";
    payload += "\"name\":\"Soil Moisture\",";
    payload += "\"state_topic\":\"plant/soil\",";
    payload += "\"unit_of_measurement\":\"%\",";
    payload += "\"device_class\":\"humidity\",";
    payload += "\"unique_id\":\"" + uniqueId + "\",";
    payload += "\"device\":{";
    payload += "\"identifiers\":[\"plant_sensor_" + mac + "\"],";
    payload += "\"name\":\"Plant Sensor " + mac + "\",";
    payload += "\"manufacturer\":\"DIY\",";
    payload += "\"model\":\"ESP32 Plant Monitor\"";
    payload += "}}";

    _mqttClient.publish(topic.c_str(), 0, true, payload.c_str());
}

void MQTTManager::publishSoil(float percent) {

    if (_mqttClient.connected()) {
        _mqttClient.publish("plant/soil", 0, false, String(percent).c_str());
    }
}