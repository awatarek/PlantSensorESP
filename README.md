# 🌿 Plant Sensor ESP32

ESP32-based smart plant monitoring device that measures soil moisture
and publishes the data via **MQTT** while also providing a **local web
dashboard** for configuration and monitoring.

The project is designed as a simple IoT sensor for house plants that
integrates easily with **Home Assistant** or other MQTT systems.

------------------------------------------------------------------------

# ✨ Features

## 🌱 Soil Moisture Monitoring

-   Reads analog soil moisture sensor
-   Converts voltage to **moisture percentage**
-   Calculates plant condition:

  Status       Description
  ------------ -----------------------------
  🌵 Dry       Soil moisture below minimum
  🌿 Optimal   Moisture within range
  💧 Wet       Moisture above maximum

------------------------------------------------------------------------

## 📡 MQTT Integration

Sensor can publish data to an MQTT broker.

Example values sent:

-   moisture percentage\
-   sensor voltage\
-   plant status

### Example topics

    plant_sensor/moisture
    plant_sensor/voltage
    plant_sensor/status

### Example payload

    moisture: 48.3
    voltage: 1.64
    status: Optimal

------------------------------------------------------------------------

## 🌐 Web Dashboard

ESP32 hosts a small web interface allowing users to:

-   view soil moisture
-   see plant status
-   configure WiFi
-   configure MQTT
-   change plant settings
-   reset configuration

### Example dashboard

    🌿 Plant Sensor

    Plant: Araucaria

    Moisture: 48%
    Status: Optimal

    WiFi: Connected
    MQTT: Connected

------------------------------------------------------------------------

## 📶 WiFi Provisioning

If the device cannot connect to WiFi it automatically starts **Access
Point mode**.

Network name:

    Plant-Sensor-Setup

Then open:

    http://10.0.0.1

Select your WiFi network and enter the password.

Credentials are stored in **ESP32 Preferences (NVS)**.

------------------------------------------------------------------------

## 🌿 Plant Configuration

Each plant has configurable parameters:

-   plant name
-   minimum moisture threshold
-   maximum moisture threshold

### Example

    Plant: Araucaria
    Min moisture: 35%
    Max moisture: 65%

### Sensor status logic

    if moisture < min → Dry
    if moisture > max → Wet
    else → Optimal

------------------------------------------------------------------------

# 💾 Persistent Storage

The device stores configuration in **ESP32 flash memory**:

-   WiFi credentials
-   MQTT settings
-   plant configuration

Data persists after reboot.

------------------------------------------------------------------------

# 🔧 Hardware

Example setup:

  Component         Example
  ----------------- ---------------------------------
  Microcontroller   ESP32 / ESP32-C3 / ESP32-C6
  Soil sensor       Capacitive Soil Moisture Sensor
  LED (optional)    WS2812B
  Power             USB-C / Li-Ion battery

### Example wiring

    Soil Sensor
    VCC -> 3.3V
    GND -> GND
    AOUT -> ESP32 ADC pin

------------------------------------------------------------------------

# 🧠 Software Architecture

### Project structure

    PlantSensorESP
    │
    ├── wifi_manager
    │   WiFi connection and AP provisioning
    │
    ├── mqtt_manager
    │   MQTT communication
    │
    ├── web_server
    │   HTTP server and routes
    │
    ├── website
    │   HTML dashboard generator
    │
    ├── soil_sensor
    │   Sensor reading and plant logic
    │
    └── PlantSensorESP.ino
        Main device logic

------------------------------------------------------------------------

# 📦 Main Components

## SoilSensor

Handles sensor reading and plant logic.

### Functions

    update()
    getVoltage()
    getPercent()
    getStatus()
    setPlantConfig()
    getPlantConfig()

------------------------------------------------------------------------

# 📄 License

MIT License

------------------------------------------------------------------------

# 👨‍💻 Author

**Bartosz Krupa**\
ESP32 Plant Sensor Project 🌿
