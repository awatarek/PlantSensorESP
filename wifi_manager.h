#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>

class WiFiManager {
public:
    WiFiManager(AsyncWebServer& server, Preferences& prefs);
    void checkNetworkChange();
    void startAP();
    bool connectWiFi();

private:
    AsyncWebServer& _server;
    Preferences& _prefs;

    String buildNetworkList();
};

#endif