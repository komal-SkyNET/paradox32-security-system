/**
 * Paradox MQTT Bridge
 * ESP32-based bridge connecting Paradox alarm systems to MQTT
 *
 * Copyright (C) 2025 Komal Venkatesh Ganesan
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <Arduino.h>
#include "Config.h"
#include "LedHandler.h"
#include "WiFiMqttConfig.h"
#include "MqttHandler.h"
#include "OtaHandler.h"
#include "ParadoxHandler.h"
#include "WebUi.h"
#include "ParadoxEvents.h"
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <LittleFS.h>

// =================================================================
// Global Objects
// =================================================================

LedHandler ledHandler(LED_PIN);
WiFiMqttConfig wifiConfig;
MqttHandler mqttHandler;
OtaHandler otaHandler;
ParadoxHandler paradoxHandler(PARADOX_SERIAL);
WebUi webUi;

// =================================================================
// Callback Functions
// =================================================================

void onParadoxEvent(const String& event, const String& payload) {
    String description = getEventDescription(event.toInt(), payload.toInt());
    if (description.length() > 0) {
        DEBUG_PRINTF("[Paradox] Event: %s\n", description.c_str());
    } else {
        DEBUG_PRINTF("[Paradox] Event: %s, Payload: %s\n", event.c_str(), payload.c_str());
    }

    String topic = String(MQTT_TOPIC_PREFIX) + "/events/" + event;
    String jsonPayload = "{\"value\":\"" + payload + "\"}";
    if (mqttHandler.publish(topic.c_str(), jsonPayload.c_str())) {
        ledHandler.setMode(LedMode::FLICKER);
    }
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
    String topicStr(topic);
    payload[length] = '\0'; // Null-terminate the payload

    DEBUG_PRINTF("[MQTT] Message received. Topic: %s, Payload: %s\n", topic, (char*)payload);
    ledHandler.setMode(LedMode::FLICKER);

    if (topicStr == String(MQTT_TOPIC_PREFIX) + "/commands") {
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error) {
            DEBUG_PRINTF("[MQTT] deserializeJson() failed: %s\n", error.c_str());
            return;
        }

        if (doc.containsKey("password")) {
            const char* password = doc["password"];
            paradoxHandler.setPassword(password);
            DEBUG_PRINTLN("[MQTT] Password updated from payload.");
        }

        uint8_t partition = doc.containsKey("partition") ? doc["partition"].as<uint8_t>() : 0;

        if (doc.containsKey("command")) {
            String command = doc["command"].as<String>();
            DEBUG_PRINTF("[MQTT] Processing command: %s\n", command.c_str());

            if (command.equalsIgnoreCase("arm")) paradoxHandler.arm(partition, 0x04);

            else if (command.equalsIgnoreCase("disarm")) paradoxHandler.disarm(partition, 0x05);
            else if (command.equalsIgnoreCase("stay")) paradoxHandler.arm(partition, 0x01);
            else if (command.equalsIgnoreCase("sleep")) paradoxHandler.arm(partition, 0x03);
            else if (command.equalsIgnoreCase("status")) paradoxHandler.requestStatus();
            else if (command.equalsIgnoreCase("status-getzones")) paradoxHandler.requestZoneStatus();
            else if (command.equalsIgnoreCase("status-getarmstatus")) paradoxHandler.requestPartitionStatus();
            else if (command.equalsIgnoreCase("disconnect")) paradoxHandler.disconnect();
        }
    }
}

// =================================================================
// Setup and Loop
// =================================================================

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 1000);
    DEBUG_PRINTLN("\n[System] Booting up Paradox MQTT Bridge v2.4...");

    pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);

    ledHandler.setup();
    wifiConfig.setup();

    if (!wifiConfig.isConfigured()) {
        DEBUG_PRINTLN("[System] Configuration is incomplete. Starting configuration portal.");
        wifiConfig.startPortal();
        DEBUG_PRINTLN("[System] Configuration saved. Rebooting to apply settings...");
        delay(1000);
        ESP.restart();
    }

    mqttHandler.setup(
        wifiConfig.getMqttServer(),
        wifiConfig.getMqttPort(),
        wifiConfig.getMqttUser(),
        wifiConfig.getMqttPassword(),
        String(MQTT_TOPIC_PREFIX) + "/commands",
        onMqttMessage
    );

    otaHandler.setup(HOSTNAME, &ledHandler);
    paradoxHandler.setup(onParadoxEvent);
    webUi.setup();

    paradoxHandler.setPassword(PARADOX_DEFAULT_PASSWORD);

    DEBUG_PRINTLN("[System] Setup complete. Running normally.");
}

void loop() {
    ledHandler.loop();
    otaHandler.loop();

    // Factory Reset Logic
    static unsigned long buttonPressStartTime = 0;
    static bool resetInProgress = false;
    const int factoryResetHoldTime = 5000; // 5 seconds

    if (digitalRead(FACTORY_RESET_PIN) == LOW) {
        if (!resetInProgress) {
            resetInProgress = true;
            buttonPressStartTime = millis();
            DEBUG_PRINTLN("[System] Factory reset button held. Keep holding for 5 seconds...");
        }
    } else {
        if (resetInProgress) {
            resetInProgress = false;
            DEBUG_PRINTLN("[System] Factory reset cancelled.");
        }
    }

    if (resetInProgress && (millis() - buttonPressStartTime > factoryResetHoldTime)) {
        DEBUG_PRINTLN("[System] Factory reset triggered! Erasing config and rebooting.");
        if (LittleFS.begin()) {
            LittleFS.remove("/config.json");
        }
        WiFiManager wm;
        wm.resetSettings();
        delay(200);
        ESP.restart();
    }

    if (!otaHandler.isOtaInProgress()) {
        mqttHandler.loop();
        paradoxHandler.loop();

        if (resetInProgress) {
            ledHandler.setMode(LedMode::BLINK_FAST);
        } else if (WiFi.isConnected()) {
            if (mqttHandler.isConnected()) {
                ledHandler.setMode(LedMode::ON);
            } else {
                ledHandler.setMode(LedMode::BLINK_SLOW);
            }
        } else {
            ledHandler.setMode(LedMode::BLINK_FAST);
        }

        static String lastWifiStatus = "";
        static String lastMqttStatus = "";
        String currentWifiStatus = WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected";
        String currentMqttStatus = mqttHandler.getConnectionStatus();

        if (currentWifiStatus != lastWifiStatus || currentMqttStatus != lastMqttStatus) {
            DEBUG_PRINTF("[System] WiFi: %s, MQTT: %s\n", currentWifiStatus.c_str(), currentMqttStatus.c_str());
            lastWifiStatus = currentWifiStatus;
            lastMqttStatus = currentMqttStatus;
        }
    }
}