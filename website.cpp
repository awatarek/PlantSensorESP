#include "website.h"
#include "soil_sensor.h"


Website::Website(AsyncMqttClient& mqttClient)
    : _mqttClient(mqttClient) {}

String Website::build(const SoilSensor& soil,
                      bool wifiConnected,
                      bool mqttConfigured,
                      bool mqttConnected,
                      const String& networkList) {

    PlantConfig plant = soil.getPlantConfig();           

    String html;
    html.reserve(4000);

    html += "<html><head><meta charset='utf-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<title>Plant Sensor</title>";
    html += "<style>";
    html += "body{font-family:Arial;text-align:center;background:#111;color:#eee}";
    html += ".card{background:#1e1e1e;padding:20px;border-radius:12px;margin:20px}";
    html += ".good{color:#4caf50}";
    html += ".dry{color:#ff9800}";
    html += ".wet{color:#2196f3}";
    html += "</style>";
    html += "</head><body>";

    html += "<h2>🌿 Plant Sensor</h2>";

    html += "<div class='card'>";
    html += "<h3>Soil Moisture</h3>";
    html += "<p style='font-size:28px'>" + String(soil.getPercent(), 1) + " %</p>";
    html += "<p>Voltage: " + String(soil.getVoltage(), 2) + " V</p>";

    String status = soil.getStatus();
    String cssClass = "good";

    if (status.indexOf("Dry") >= 0) cssClass = "dry";
    if (status.indexOf("Wet") >= 0) cssClass = "wet";

    html += "<p class='" + cssClass + "'>" + status + "</p>";
    html += "</div>";


    html += "<div class='card'>";
    html += "<h3>Plant Settings</h3>";

    html += "<p>Current name: <b>";
    html += plant.name;
    html += "</b></p>";

    html += "<p>Current min: <b>";
    html += String(plant.minMoisture, 1);
    html += " %</b></p>";

    html += "<p>Current max: <b>";
    html += String(plant.maxMoisture, 1);
    html += " %</b></p>";

    html += "<form method='POST' action='/save_plant'>";

    html += "Plant name<br>";
    html += "<input name='name' value='" + plant.name + "'><br>";

    html += "Min moisture (%)<br>";
    html += "<input name='min' type='number' value='" + String(plant.minMoisture, 1) + "'><br>";

    html += "Max moisture (%)<br>";
    html += "<input name='max' type='number' value='" + String(plant.maxMoisture, 1) + "'><br><br>";

    html += "<input type='submit' value='Save Plant'>";
    html += "</form>";

    html += "</div>";


    html += "<div class='card'>";
    html += "<h3>Sensor calibration</h3>";

    html += "<button onclick=\"fetch('/calibrateDry')\">";
    html += "Set DRY (sensor in air)";
    html += "</button>";

    html += "<button onclick=\"fetch('/calibrateWet')\">";
    html += "Set WET (sensor in water)";
    html += "</button>";
    html += "</div>";


    if (!wifiConnected) {

        html += "<div class='card'>";
        html += "<h3>WiFi Setup</h3>";
        html += "<form method='POST' action='/save_wifi'>";

        html += "<h4>Select Network:</h4>";
        html += networkList;

        html += "<br>Password:<br>";
        html += "<input name='pass' type='password'><br><br>";
        html += "<input type='submit' value='Connect'>";
        html += "</form>";
        html += "</div>";

    } else {

        html += "<p>WiFi Connected ✅</p>";

        if (!mqttConfigured) {

            html += "<div class='card'>";
            html += "<h3>MQTT Setup</h3>";
            html += "<form method='POST' action='/save_mqtt'>";
            html += "Broker:<br><input name='broker'><br>";
            html += "Username:<br><input name='user'><br>";
            html += "Password:<br><input name='pass' type='password'><br><br>";
            html += "<input type='submit' value='Save MQTT'>";
            html += "</form>";
            html += "</div>";

        } else {

            if (mqttConnected)
                html += "<p>MQTT Connected 🟢</p>";
            else
                html += "<p>MQTT Disconnected 🔴</p>";
        }
    }

    html += "</body></html>";
    return html;
}