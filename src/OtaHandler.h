#pragma once
#include <Arduino.h>

// Forward declare LedHandler to avoid circular dependency
class LedHandler;

class OtaHandler {
public:
    void setup(const char* hostname, LedHandler* ledHandler);
    void loop();
    bool isOtaInProgress() const;

private:
    bool _otaInProgress = false;
};

