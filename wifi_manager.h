#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>

class WiFiManager {
public:
    WiFiManager(AsyncWebServer& server, Preferences& prefs);

    void checkNetworkChange();
    void startAP();
    bool connectWiFi();
    bool isConnected();
    void saveCredentials(const String& ssid,
                        const String& pass);
    void clearCredentials();    
    String buildNetworkList();

private:
    AsyncWebServer& _server;
    Preferences& _prefs;

};

#endif