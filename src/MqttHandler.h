#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <functional>

// Define the function signature for the message callback
using MqttCallback = std::function<void(char*, byte*, unsigned int)>;

class MqttHandler {
public:
    MqttHandler();
    void setup(const char* server, int port, const char* user, const char* password, const String& commandTopic, MqttCallback callback);
    void loop();
    bool publish(const char* topic, const char* payload);
    bool isConnected();
    const char* getConnectionStatus();

private:
    WiFiClient _wifiClient;
    PubSubClient _mqttClient;
    String _server;
    int _port;
    String _user;
    String _password;
    String _commandTopic;
    unsigned long _lastReconnectAttempt = 0;
    bool _statusMessageSent = false;

    void reconnect();
};