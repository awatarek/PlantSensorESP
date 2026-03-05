#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncMqttClient.h>
#include <Preferences.h>

#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "website.h"
#include "soil_sensor.h"
#include "web_server.h"

#define SOIL_PIN D0

Preferences prefs;
AsyncWebServer server(80);
AsyncMqttClient mqttClient;

WiFiManager wifiManager(server, prefs);
MQTTManager mqttManager(server, mqttClient, prefs);
Website website(mqttClient);
SoilSensor soil(SOIL_PIN);
WebServerManager webServer(server, wifiManager, mqttManager, website, soil);

void setup() {
  Serial.begin(115200);
  delay(3500);
  
  Serial.println();
  Serial.println("===BOOT===");
  Serial.print("=== ");
  Serial.print(__DATE__);
  Serial.print(" ");
  Serial.print(__TIME__);
  Serial.println(" ===");
  Serial.println();

  prefs.begin("plant", true);

  PlantConfig plant;
  plant.name = prefs.getString("name", "Plant");
  plant.minMoisture = prefs.getFloat("min", 30);
  plant.maxMoisture = prefs.getFloat("max", 70);
  plant.dryVoltage = prefs.getFloat("dry", 2.2);
  plant.wetVoltage = prefs.getFloat("wet", 0.9);

  prefs.end();

  soil.setPlantConfig(plant);

  bool wifiOk = wifiManager.connectWiFi();

  if (!wifiOk) {
    wifiManager.startAP();
  } else {
    wifiManager.checkNetworkChange();
  }

  mqttManager.begin();

  webServer.begin();

  server.begin();
}

/* ============================================================
   LOOP
============================================================ */

void loop() {
  static unsigned long last = 0;
  if (millis() - last > 5000) {

      soil.update();

      mqttManager.publishValue("soil", String(soil.getPercent()));
      mqttManager.publishValue("voltage", String(soil.getVoltage()));
      mqttManager.publishValue("status", soil.getStatus());

      last = millis();
  }


  static unsigned long lastWifiCheck = 0;

  if (millis() - lastWifiCheck > 15000) {
      lastWifiCheck = millis();

      if (WiFi.status() == WL_CONNECTED) {
          Serial.print("WiFi RSSI: ");
          Serial.print(WiFi.RSSI());
          Serial.println(" dBm");
      }
  }
}