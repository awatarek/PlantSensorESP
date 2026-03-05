#include "web_server.h"
#include <Preferences.h>

extern Preferences prefs;
WebServerManager::WebServerManager(AsyncWebServer& server,
                                   WiFiManager& wifi,
                                   MQTTManager& mqtt,
                                   Website& website,
                                   SoilSensor& soil)
    : _server(server),
      _wifi(wifi),
      _mqtt(mqtt),
      _website(website),
      _soil(soil) {}

void WebServerManager::begin() {

    setupRoutes();
    _server.begin();
}

void WebServerManager::setupRoutes() {

    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {

        _soil.update();

        bool wifiConnected = _wifi.isConnected();
        bool mqttConfigured = _mqtt.isConfigured();
        bool mqttConnected  = _mqtt.isConnected();

        String networkList = "";

        if (!wifiConnected) {
            networkList = _wifi.buildNetworkList();
        }

        request->send(200, "text/html",
            _website.build(_soil,
                           wifiConnected,
                           mqttConfigured,
                           mqttConnected,
                           networkList));
    });

    _server.on("/save_plant", HTTP_POST, [this](AsyncWebServerRequest *request) {

        PlantConfig plant = _soil.getPlantConfig();

        prefs.begin("plant", false);

        if (request->hasParam("name", true)) {
            String name = request->getParam("name", true)->value();
            if (name.length() > 0) {
                plant.name = name;
                prefs.putString("name", name);
            }
        }

        if (request->hasParam("min", true)) {
            String val = request->getParam("min", true)->value();
            if (val.length() > 0) {
                plant.minMoisture = val.toFloat();
                prefs.putFloat("min", plant.minMoisture);
            }
        }

        if (request->hasParam("max", true)) {
            String val = request->getParam("max", true)->value();
            if (val.length() > 0) {
                plant.maxMoisture = val.toFloat();
                prefs.putFloat("max", plant.maxMoisture);
            }
        }

        prefs.end();

        _soil.setPlantConfig(plant);

        request->send(200, "text/html",
            "<h3>Plant settings saved.</h3><a href='/'>Back</a>"
            "<script>setTimeout(()=>location.href='/',1500)</script>");
    });

    _server.on("/save_wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {

        if (!request->hasParam("ssid", true) ||
            !request->hasParam("pass", true)) {

            request->send(400, "text/plain", "Missing parameters");
            return;
        }

        String ssid = request->getParam("ssid", true)->value();
        String pass = request->getParam("pass", true)->value();

        _wifi.saveCredentials(ssid, pass);

        delay(1000);
        WiFi.disconnect();
        WiFi.begin(ssid.c_str(), pass.c_str());


        Serial.println("Connecting to WiFi...");

        unsigned long start = millis();

        while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
            delay(200);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {

            Serial.println("\nWiFi connected");

            String ip = WiFi.localIP().toString();

            String html;
            html += "<h3>WiFi connected.</h3>";
            html += "<p>Device IP: <b>";
            html += ip;
            html += "</b></p>";
            html += "<p>Redirecting...</p>";
            html += "<script>";
            html += "setTimeout(()=>location.href='http://";
            html += ip;
            html += "',2000)";
            html += "</script>";

            request->send(200, "text/html", html);

            delay(2000);
            ESP.restart();
        }
        else {

            Serial.println("\nWiFi connection failed");

            request->send(200, "text/html",
                "<h3>WiFi connection failed!</h3>"
                "<a href='/'>Back</a>"
                "<script>setTimeout(()=>location.href='/',1500)</script>");
        }
    });

    _server.on("/calibrateDry", HTTP_GET, [&](AsyncWebServerRequest *request) {
        float voltage = _soil.getVoltage();

        PlantConfig plant = _soil.getPlantConfig();
        plant.dryVoltage = voltage;

        prefs.begin("plant", false);
        prefs.putFloat("dry", voltage);
        prefs.end();

        _soil.setPlantConfig(plant);

        request->send(200, "text/plain", "Dry calibrated");
    });

    _server.on("/calibrateWet", HTTP_GET, [&](AsyncWebServerRequest *request) {
        float voltage = _soil.getVoltage();
        PlantConfig plant = _soil.getPlantConfig();
        plant.wetVoltage = voltage;

        prefs.begin("plant", false);
        prefs.putFloat("wet", voltage);
        prefs.end();
        _soil.setPlantConfig(plant);

        request->send(200, "text/plain", "Wet calibrated");
    });


    _server.on("/save_mqtt", HTTP_POST, [this](AsyncWebServerRequest *request) {

        if (!request->hasParam("broker", true)) {
            request->send(400, "text/plain", "Missing broker");
            return;
        }

        String broker = request->getParam("broker", true)->value();
        String user   = request->getParam("user", true)->value();
        String pass   = request->getParam("pass", true)->value();

        _mqtt.saveCredentials(broker, user, pass);

        delay(1000);
        _mqtt.disconnect();
        _mqtt.begin();

        request->send(200, "text/html",
            "<h3>MQTT settings saved.</h3><a href='/'>Back</a>"
            "<script>setTimeout(()=>location.href='/',1500)</script>");
    });
}