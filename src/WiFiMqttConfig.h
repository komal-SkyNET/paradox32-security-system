#pragma once

#include <Arduino.h>
#include <WiFiManager.h>
#include "LedHandler.h" // Include for the reset method

class WiFiMqttConfig {
public:
    WiFiMqttConfig();
    void setup();
    void startPortal();
    bool isConfigured() const { return _isConfigured; }

    // Getters for saved values
    const char* getMqttServer() const;
    int getMqttPort() const;
    const char* getMqttUser() const;
    const char* getMqttPassword() const;
    void saveConfigCallback();


private:
    bool loadConfiguration();
    void saveConfiguration();
    bool _isConfigured = false;
    char _mqttServer[64];
    char _mqttPort[6];
    char _mqttUser[32];
    char _mqttPassword[64];

    // WiFiManager and parameters are now class members
    WiFiManager _wm;
    WiFiManagerParameter _custom_mqtt_server;
    WiFiManagerParameter _custom_mqtt_port;
    WiFiManagerParameter _custom_mqtt_user;
    WiFiManagerParameter _custom_mqtt_password;
};
