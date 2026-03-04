#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <ESPAsyncWebServer.h>
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "website.h"
#include "soil_sensor.h"

class WebServerManager {

public:
    WebServerManager(AsyncWebServer& server,
                     WiFiManager& wifi,
                     MQTTManager& mqtt,
                     Website& website,
                     SoilSensor& soil);

    void begin();

private:
    AsyncWebServer& _server;
    WiFiManager& _wifi;
    MQTTManager& _mqtt;
    Website& _website;
    SoilSensor& _soil;

    void setupRoutes();
};

#endif