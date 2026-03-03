#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncMqttClient.h>
#include <Preferences.h>

#include "wifi_manager.h"
#include "mqtt_manager.h"

#define SOIL_PIN 35

Preferences prefs;
AsyncWebServer server(80);
AsyncMqttClient mqttClient;

WiFiManager wifiManager(server, prefs);
MQTTManager mqttManager(server, mqttClient, prefs);

void setup() {
  Serial.begin(115200);

  prefs.begin("config", true);
  String wifi_ssid = prefs.getString("ssid", "");
  prefs.end();

  if (wifi_ssid == "" || !wifiManager.connectWiFi()) {
    wifiManager.startAP();
    server.begin();
    return;
  }

  wifiManager.checkNetworkChange();

  prefs.begin("config", true);
  String broker = prefs.getString("broker", "");
  prefs.end();

  if (broker == "") {
    mqttManager.registerSetupPage();
    server.begin();
    return;
  }

  mqttManager.begin();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html",
      "<h2>Plant Sensor Running</h2>");
  });

  server.begin();
}

/* ============================================================
   LOOP
============================================================ */

void loop() {
    float voltage = analogRead(SOIL_PIN) * (3.3 / 4095.0);
    float percent = (2.2 - voltage) * (100.0 / (2.2 - 0.89));
    percent = constrain(percent, 0, 100);

    mqttManager.publishSoil(percent);
    delay(5000);
}