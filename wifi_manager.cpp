#include "wifi_manager.h"

WiFiManager::WiFiManager(AsyncWebServer& server, Preferences& prefs)
    : _server(server), _prefs(prefs) {}

String WiFiManager::buildNetworkList() {

    Serial.println("Scanning WiFi...");
    int n = WiFi.scanNetworks(false, true);

    String networkList = "";

    if (n <= 0) {
        networkList = "<p>No networks found</p>";
    } else {
        for (int i = 0; i < n; ++i) {
            networkList += "<input type='radio' name='ssid' value='" + WiFi.SSID(i) + "'>";
            networkList += WiFi.SSID(i);
            networkList += " (" + String(WiFi.RSSI(i)) + " dBm)<br>";
        }
    }

    WiFi.scanDelete();
    return networkList;
}

void WiFiManager::checkNetworkChange() {

    String currentBSSID = WiFi.BSSIDstr();

    _prefs.begin("config", false);
    String lastBSSID = _prefs.getString("last_bssid", "");

    if (lastBSSID != "" && lastBSSID != currentBSSID) {
        Serial.println("Network changed (BSSID)! Clearing MQTT...");

        _prefs.putString("broker", "");
        _prefs.putString("muser", "");
        _prefs.putString("mpass", "");
    }

    _prefs.putString("last_bssid", currentBSSID);
    _prefs.end();
}


void WiFiManager::startAP() {
    Serial.println("Starting AP mode...");

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("Plant-Sensor-Setup", "plantsetup");
    WiFi.softAPConfig(IPAddress(10,0,0,1),
                      IPAddress(10,0,0,1),
                      IPAddress(255,255,255,0));

    delay(500);

    String networkList = buildNetworkList();

    _server.on("/", HTTP_GET, [this, networkList](AsyncWebServerRequest *request) {
        request->send(200, "text/html",
            "<h2>WiFi Setup</h2>"
            "<form method='POST' action='/save_wifi'>"
            "<h3>Select Network:</h3>" +
            networkList +
            "<br>Password:<br>"
            "<input name='pass' type='password'><br><br>"
            "<input type='submit' value='Connect'>"
            "</form>");
    });

    _server.on("/save_wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {

        if (!request->hasParam("ssid", true) || !request->hasParam("pass", true)) {
            request->send(400, "text/html", "<h3>Missing parameters</h3>");
            return;
        }

        String new_ssid = request->getParam("ssid", true)->value();
        String new_pass = request->getParam("pass", true)->value();

        request->send(200, "text/html",
            "<h3>Connecting to WiFi...</h3>"
            "<p>Please wait...</p>");

        Serial.println("Selected SSID: " + new_ssid);

        _prefs.begin("config", false);
        _prefs.putString("ssid", new_ssid);
        _prefs.putString("wpass", new_pass);
        _prefs.end();

        delay(1000);
        ESP.restart();
    });

    _server.begin();
}

bool WiFiManager::connectWiFi() {

    Serial.println("Connecting to WiFi...");

    _prefs.begin("config", true);
    String ssid = _prefs.getString("ssid", "");
    String pass = _prefs.getString("wpass", "");
    _prefs.end();

    if (ssid == "") {
        Serial.println("No saved WiFi credentials.");
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());

    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected!");
        Serial.println(WiFi.localIP());
        return true;
    }

    Serial.println("\nWiFi connection failed.");
    return false;
}