#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncMqttClient.h>
#include <Preferences.h>

#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "website.h"
#include "soil_sensor.h"

#define SOIL_PIN 8

Preferences prefs;
AsyncWebServer server(80);
AsyncMqttClient mqttClient;

WiFiManager wifiManager(server, prefs);
MQTTManager mqttManager(server, mqttClient, prefs);
Website website(mqttClient);
SoilSensor soil(SOIL_PIN);

void setup() {
  Serial.begin(115200);
  delay(1500); 
  
  Serial.println();
  Serial.println("===BOOT===");
  Serial.print("=== ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.print(__TIME__);
  Serial.println(" ===");
  Serial.println();

  // 1️⃣ Spróbuj połączyć WiFi
  bool wifiOk = wifiManager.connectWiFi();

  if (!wifiOk) {
    wifiManager.startAP();
  } else {
    wifiManager.checkNetworkChange();
  }

  // 2️⃣ Uruchom MQTT (sam sprawdzi czy jest skonfigurowany)
  mqttManager.begin();

  // 3️⃣ Jeden centralny route "/"
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {

      soil.update();

      bool wifiConnected = wifiManager.isConnected();
      bool mqttConfigured = mqttManager.isConfigured();
      bool mqttConnected  = mqttManager.isConnected();

      String networkList = "";

      if (!wifiConnected) {
          networkList = wifiManager.buildNetworkList();
      }

      request->send(200, "text/html",
          website.build(soil,
                        wifiConnected,
                        mqttConfigured,
                        mqttConnected,
                        networkList));
  });

  server.on("/save_wifi", HTTP_POST, [](AsyncWebServerRequest *request) {

    if (!request->hasParam("ssid", true) ||
        !request->hasParam("pass", true)) {
        request->send(400, "text/plain", "Missing parameters");
        return;
    }

    String ssid = request->getParam("ssid", true)->value();
    String pass = request->getParam("pass", true)->value();

    wifiManager.saveCredentials(ssid, pass);

    request->send(200, "text/html",
        "<h3>WiFi Saved. Rebooting...</h3>");

    delay(1000);
    ESP.restart();
  });

  server.on("/save_mqtt", HTTP_POST, [](AsyncWebServerRequest *request) {

    if (!request->hasParam("broker", true)) {
        request->send(400, "text/plain", "Missing broker");
        return;
    }

    String broker = request->getParam("broker", true)->value();
    String user   = request->getParam("user", true)->value();
    String pass   = request->getParam("pass", true)->value();



    mqttManager.saveCredentials(broker, user, pass);


    request->send(200, "text/html",
        "<h3>MQTT Saved. Rebooting...</h3>");

    delay(1000);
    ESP.restart();
  });


  server.begin();
}

/* ============================================================
   LOOP
============================================================ */

void loop() {
    soil.update();

    mqttManager.publishValue("soil", String(soil.getPercent()));
    mqttManager.publishValue("voltage", String(soil.getVoltage()));
    mqttManager.publishValue("status", soil.getStatus());
    delay(5000);
}