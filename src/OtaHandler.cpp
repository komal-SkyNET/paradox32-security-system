#include "OtaHandler.h"
#include "Config.h"
#include "LedHandler.h" // Include the full header here
#include <ArduinoOTA.h>

void OtaHandler::setup(const char* hostname, LedHandler* ledHandler) {
    ArduinoOTA.setHostname(hostname);
    // ArduinoOTA.setPassword("your_password"); // Optional password protection

    ArduinoOTA.onStart([this, ledHandler]() {
        _otaInProgress = true; // Set flag
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        DEBUG_PRINTLN("[OTA] Start updating " + type);
        if (ledHandler) {
            ledHandler->setMode(LedMode::BLINK_FAST);
        }
    });

    ArduinoOTA.onEnd([this]() {
        _otaInProgress = false;
        DEBUG_PRINTLN("\n[OTA] End");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        DEBUG_PRINTF("[OTA] Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([this](ota_error_t error) {
        _otaInProgress = false;
        DEBUG_PRINTF("[OTA] Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
        else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
    });

    ArduinoOTA.begin();
    DEBUG_PRINTLN("[OTA] Handler ready.");
    DEBUG_PRINTF("[OTA] Hostname: %s\n", hostname);
}

void OtaHandler::loop() {
    ArduinoOTA.handle();
}

bool OtaHandler::isOtaInProgress() const {
    return _otaInProgress;
}

