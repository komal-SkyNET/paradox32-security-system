#pragma once

#include <Arduino.h>
#include "Log.h" // Central logging function

// =================================================================
// Debugging
// =================================================================
// Comment out to disable serial debug messages and save space
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_PRINT(x) Log(x)
  #define DEBUG_PRINTLN(x) Log("%s\n", x)
  #define DEBUG_PRINTF(fmt, ...) Log(fmt, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(fmt, ...)
#endif

// =================================================================
// Hardware Pins
// =================================================================
#define LED_PIN 2 // Onboard LED for most ESP32 dev boards

// Define the Serial port for communication with the Paradox Panel
// Use Serial2 by default for ESP32.
#define PARADOX_SERIAL Serial2
#define PARADOX_RX_PIN 16
#define PARADOX_TX_PIN 17
#define PARADOX_BAUD_RATE 9600
#define PARADOX_DEFAULT_PASSWORD 0000 // Password/4 digit pin used in serial comms to Paradox panel 

// =================================================================
// Factory Reset Configuration
// =================================================================
// Ground this pin during boot to wipe saved credentials.
// Pin 0 is often the "BOOT" button on dev boards.
#define FACTORY_RESET_PIN 0

// =================================================================
// Network & OTA Configuration
// =================================================================
#define HOSTNAME "paradox-mqtt-bridge"
#define FIRMWARE_VERSION "2.7"
#define CONFIG_PORTAL_SSID "ParadoxConfig"
#define CONFIG_PORTAL_PASSWORD "paradox123"

// =================================================================
// MQTT Configuration
// =================================================================
#define MQTT_TOPIC_PREFIX "paradox"
#define MQTT_CLIENT_ID "ParadoxBridgeESP32"
#define MQTT_DEFAULT_PORT 1883
#define MQTT_RECONNECT_DELAY 5000 // 5 seconds

// Subscribe to a topic for receiving commands
// The wildcard '#' means we subscribe to all sub-topics under 'commands'
#define MQTT_COMMAND_TOPIC "paradox/commands/#"

// =================================================================
// Paradox Handler Configuration
// =================================================================
#define PARADOX_MSG_BUFFER_SIZE 32 // Max size of a message from the panel
#define PARADOX_READ_TIMEOUT 100 // ms to wait for a full message
#define PARADOX_KEEP_ALIVE_INTERVAL 60000 // 60 seconds. Send a status request to keep the session alive.
