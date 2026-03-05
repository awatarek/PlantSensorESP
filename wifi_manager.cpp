#include "wifi_manager.h"

#if defined(ESP32)
#include "esp_wifi.h"
#endif

WiFiManager::WiFiManager(AsyncWebServer& server, Preferences& prefs)
    : _server(server), _prefs(prefs) {}

String WiFiManager::buildNetworkList() {

    Serial.println("Scanning WiFi...");

    wifi_mode_t previousMode = WiFi.getMode();

    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    delay(100);

    int n = WiFi.scanNetworks(false, true, false, 300);

    if (n <= 0) {
        WiFi.mode(previousMode);
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
        if (rssi < -95) continue;

        String ssid = WiFi.SSID(i);
        if (ssid.length() == 0) continue;

        bool secure = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;

        networks.push_back({ssid, rssi, secure});
    }

    WiFi.scanDelete();
    WiFi.mode(previousMode);

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

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_OFF);

    delay(200);

#if defined(ESP32)

    wifi_country_t country = {
        .cc = "PL",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_AUTO
    };

    esp_wifi_set_country(&country);

#endif

    WiFi.mode(WIFI_AP);
    delay(100);
    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);

    WiFi.softAPConfig(
        IPAddress(10,0,0,1),
        IPAddress(10,0,0,1),
        IPAddress(255,255,255,0)
    );

    WiFi.softAP("Plant-Sensor-Setup", "plantsetup", 6, false, 4);

    delay(200);

    Serial.println("AP started");
    Serial.print("SSID: ");
    Serial.println(WiFi.softAPSSID());

    Serial.print("IP: ");
    Serial.println(WiFi.softAPIP());

    Serial.print("Stations: ");
    Serial.println(WiFi.softAPgetStationNum());
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

    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(false);

#if defined(ESP32)

    wifi_country_t country = {
        .cc = "PL",
        .schan = 1,
        .nchan = 13,
        .policy = WIFI_COUNTRY_POLICY_AUTO
    };

    esp_wifi_set_country(&country);

    esp_wifi_set_protocol(
        WIFI_IF_STA,
        WIFI_PROTOCOL_11B |
        WIFI_PROTOCOL_11G |
        WIFI_PROTOCOL_11N
    );

#endif

    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 15000) {

        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {

        WiFi.softAPdisconnect(true);
        WiFi.enableAP(false);

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