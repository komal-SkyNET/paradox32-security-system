#include "MqttHandler.h"
#include "ParadoxHandler.h"
#include "Config.h"
#include <ArduinoJson.h>

MqttHandler::MqttHandler() : _mqttClient(_wifiClient) {
    _statusMessageSent = false;
}

void MqttHandler::setup(const char* server, int port, const char* user, const char* password, const String& commandTopic, MqttCallback callback) {
    _server = server;
    _port = port;
    _user = user;
    _password = password;
    _commandTopic = commandTopic;

    _mqttClient.setServer(_server.c_str(), _port);
    _mqttClient.setCallback(callback);
    DEBUG_PRINTF("[MQTT] Handler setup for server %s:%d\n", _server.c_str(), _port);
}

bool MqttHandler::isConnected() {
    return _mqttClient.connected();
}

const char* MqttHandler::getConnectionStatus() {
    if (_mqttClient.connected()) {
        return "Connected";
    }

    if (_lastReconnectAttempt == 0) {
        return "Disconnected";
    }

    int state = _mqttClient.state();
    if (state == -4 || state == -3) { // MQTT_CONNECTION_TIMEOUT or MQTT_CONNECTION_LOST
        return "Connection Lost";
    }

    return "Connecting";
}

void MqttHandler::loop() {
    if (!isConnected()) {
        _statusMessageSent = false; // Reset on disconnect
        unsigned long now = millis();
        if (now - _lastReconnectAttempt > MQTT_RECONNECT_DELAY) {
            _lastReconnectAttempt = now;
            reconnect();
        }
    } else {
        _mqttClient.loop();
    }
}

void MqttHandler::reconnect() {
    if (_server.length() == 0) {
        return;
    }
    DEBUG_PRINT("[MQTT] Attempting to connect... ");
    if (_mqttClient.connect(MQTT_CLIENT_ID, _user.c_str(), _password.c_str())) {
        DEBUG_PRINTLN("connected!");
        _mqttClient.subscribe(_commandTopic.c_str());
        DEBUG_PRINTF("[MQTT] Subscribed to: %s\n", _commandTopic.c_str());

        if (!_statusMessageSent) {
            StaticJsonDocument<256> doc;
            doc["firmware_version"] = FIRMWARE_VERSION;
            doc["wifi_status"] = "Connected";
            doc["mqtt_status"] = "Connected";
            char payload[256];
            serializeJson(doc, payload);
            publish("paradox/__status__", payload);
            _statusMessageSent = true;

            // Request a full status update now that we are connected
            DEBUG_PRINTLN("[MQTT] Requesting initial zone and partition status.");
            paradoxHandler.requestZoneStatus();
            delay(500); // Add a small delay between requests
            paradoxHandler.requestPartitionStatus();
        }
    } else {
        DEBUG_PRINTF("failed, rc=%d. Retrying in 5 seconds.\n", _mqttClient.state());
    }
}

bool MqttHandler::publish(const char* topic, const char* payload) {
    if (isConnected()) {
        DEBUG_PRINTF("[MQTT] Publishing. Topic: %s, Payload: %s\n", topic, payload);
        return _mqttClient.publish(topic, payload, true); // Retain message
    }
    DEBUG_PRINTLN("[MQTT] Cannot publish, not connected.");
    return false;
}