#include "WiFiMqttConfig.h"
#include "Config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// Global instance pointer for the callback
WiFiMqttConfig* instance = nullptr;

void saveConfigCallbackGlobal() {
    if (instance) {
        instance->saveConfigCallback();
    }
}

// Private method to load configuration from LittleFS
bool WiFiMqttConfig::loadConfiguration() {
    DEBUG_PRINTLN("[FS] Attempting to load configuration...");
    if (!LittleFS.begin(true)) { // Format on fail
        DEBUG_PRINTLN("[FS] Failed to mount file system.");
        return false;
    }

    File configFile = LittleFS.open("/config.json", "r");
    if (!configFile) {
        DEBUG_PRINTLN("[FS] Failed to open config file for reading.");
        return false;
    }

    size_t size = configFile.size();
    std::unique_ptr<char[]> buf(new char[size + 1]);
    configFile.readBytes(buf.get(), size);
    buf[size] = '\0'; // Null-terminate the buffer

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, buf.get());
    configFile.close();

    if (error) {
        DEBUG_PRINTF("[FS] Failed to parse config file: %s\n", error.c_str());
        return false;
    }

    // First, copy all values from the JSON document to our variables
    strlcpy(_mqttServer, doc["server"] | "", sizeof(_mqttServer));
    strlcpy(_mqttPort, doc["port"] | "1883", sizeof(_mqttPort));
    strlcpy(_mqttUser, doc["user"] | "", sizeof(_mqttUser));
    strlcpy(_mqttPassword, doc["password"] | "", sizeof(_mqttPassword));

    // Second, for security, remove the password from the JSON object before logging it
    doc.as<JsonObject>().remove("password");
    char sanitized_buf[256];
    serializeJson(doc, sanitized_buf);
    DEBUG_PRINTLN("[FS] --- SANITIZED CONFIG FILE CONTENT ---");
    DEBUG_PRINTLN(sanitized_buf);
    DEBUG_PRINTLN("[FS] --- END SANITIZED CONFIG FILE CONTENT ---");

    DEBUG_PRINTLN("[FS] Configuration loaded successfully from file.");
    DEBUG_PRINTF("[FS] Loaded Server: '%s'\n", _mqttServer);
    return true;
}

// Private method to save configuration to LittleFS
void WiFiMqttConfig::saveConfiguration() {
    DEBUG_PRINTLN("[FS] Saving configuration...");
    if (!LittleFS.begin(true)) { // Format on fail
        DEBUG_PRINTLN("[FS] Failed to mount file system for saving.");
        return;
    }

    StaticJsonDocument<256> doc;
    doc["server"] = _custom_mqtt_server.getValue();
    doc["port"] = _custom_mqtt_port.getValue();
    doc["user"] = _custom_mqtt_user.getValue();
    doc["password"] = _custom_mqtt_password.getValue();

    File configFile = LittleFS.open("/config.json", "w");
    if (!configFile) {
        DEBUG_PRINTLN("[FS] Failed to open config file for writing.");
        return;
    }

    if (serializeJson(doc, configFile) == 0) {
        DEBUG_PRINTLN("[FS] Failed to write to config file.");
    }
    configFile.close();
    DEBUG_PRINTLN("[FS] Configuration saved.");
}

WiFiMqttConfig::WiFiMqttConfig() :
    _custom_mqtt_server("server", "MQTT Server", _mqttServer, 64),
    _custom_mqtt_port("port", "MQTT Port", _mqttPort, 6, "1883"),
    _custom_mqtt_user("user", "MQTT User", _mqttUser, 32),
    _custom_mqtt_password("password", "MQTT Password", _mqttPassword, 64)
{
    _mqttServer[0] = '\0';
    _mqttPort[0] = '\0';
    _mqttUser[0] = '\0';
    _mqttPassword[0] = '\0';
    instance = this;
}

void WiFiMqttConfig::setup() {
    loadConfiguration();

    _custom_mqtt_server.setValue(_mqttServer, 64);
    _custom_mqtt_port.setValue(_mqttPort, 6);
    _custom_mqtt_user.setValue(_mqttUser, 32);
    _custom_mqtt_password.setValue(_mqttPassword, 64);

    _wm.addParameter(&_custom_mqtt_server);
    _wm.addParameter(&_custom_mqtt_port);
    _wm.addParameter(&_custom_mqtt_user);
    _wm.addParameter(&_custom_mqtt_password);
    _wm.setSaveConfigCallback(saveConfigCallbackGlobal);
    _wm.setConnectTimeout(30);
    _wm.setConfigPortalTimeout(1);

    _wm.autoConnect(CONFIG_PORTAL_SSID, CONFIG_PORTAL_PASSWORD);

    if (strlen(_mqttServer) > 0) {
        _isConfigured = true;
        DEBUG_PRINTLN("[WiFi] Configuration is valid.");
    } else {
        _isConfigured = false;
        DEBUG_PRINTLN("[WiFi] Configuration is incomplete.");
    }
}

void WiFiMqttConfig::startPortal() {
    DEBUG_PRINTLN("[WiFi] Starting configuration portal.");
    _wm.setConfigPortalTimeout(180);
    if (!_wm.startConfigPortal(CONFIG_PORTAL_SSID, CONFIG_PORTAL_PASSWORD)) {
        DEBUG_PRINTLN("[WiFi] Portal timed out. Rebooting.");
        delay(1000);
        ESP.restart();
    }
}

void WiFiMqttConfig::saveConfigCallback() {
    DEBUG_PRINTLN("[WiFi] Save callback triggered. Saving config now.");
    saveConfiguration();
}

const char* WiFiMqttConfig::getMqttServer() const { return _mqttServer; }
const char* WiFiMqttConfig::getMqttUser() const { return _mqttUser; }
const char* WiFiMqttConfig::getMqttPassword() const { return _mqttPassword; }
int WiFiMqttConfig::getMqttPort() const {
    int port = atoi(_mqttPort);
    return (port == 0) ? MQTT_DEFAULT_PORT : port;
}
