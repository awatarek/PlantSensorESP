#include "wifi_manager.h"
#if defined(ESP32)
  #include "esp_wifi.h"
#endif

WiFiManager::WiFiManager(AsyncWebServer& server, Preferences& prefs)
    : _server(server), _prefs(prefs) {}

String WiFiManager::buildNetworkList() {

    Serial.println("Scanning WiFi...");
    int n = WiFi.scanNetworks(false, true);

    if (n <= 0) {
        return "<p>No networks found</p>";
    }

    struct Network {
        String ssid;
        int rssi;
        bool secure;
    };

    std::vector<Network> networks;

    for (int i = 0; i < n; i++) {

        int rssi = WiFi.RSSI(i);

        if (rssi < -80) continue;

        String ssid = WiFi.SSID(i);

        if (ssid.length() == 0) continue;

        bool secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

        networks.push_back({ssid, rssi, secure});
    }

    WiFi.scanDelete();

    std::sort(networks.begin(), networks.end(),
        [](const Network &a, const Network &b) {
            return a.rssi > b.rssi;
        });

    String html;
    html.reserve(1500);

    for (auto &net : networks) {

        html += "<label>";
        html += "<input type='radio' name='ssid' value='" + net.ssid + "'>";

        html += net.ssid;

        if (net.secure)
            html += " 🔒";

        html += " (" + String(net.rssi) + " dBm)";
        html += "</label><br>";
    }

    return html;
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
    WiFi.mode(WIFI_AP);

    #ifdef esp_wifi_set_country

        wifi_country_t country = {
            .cc = "PL",
            .schan = 1,
            .nchan = 13,
            .policy = WIFI_COUNTRY_POLICY_AUTO
        };

        esp_wifi_set_country(&country);

    #endif

    WiFi.softAP("Plant-Sensor-Setup", "plantsetup", 6, false, 4);    
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.softAPConfig(
        IPAddress(10,0,0,1),
        IPAddress(10,0,0,1),
        IPAddress(255,255,255,0)
    );

    Serial.println("AP started.");
    Serial.println(WiFi.softAPIP());
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

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 15000) {
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

void WiFiManager::saveCredentials(const String& ssid,
                                  const String& pass) {

    _prefs.begin("config", false);
    _prefs.putString("ssid", ssid);
    _prefs.putString("wpass", pass);
    _prefs.end();
}

void WiFiManager::clearCredentials() {

    _prefs.begin("config", false);
    _prefs.remove("ssid");
    _prefs.remove("wpass");
    _prefs.end();
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}