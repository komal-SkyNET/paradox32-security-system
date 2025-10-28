#pragma once
#include <Arduino.h>

enum class LedMode {
    OFF,
    ON,
    BLINK_SLOW,
    BLINK_FAST,
    FLICKER
};

class LedHandler {
public:
    explicit LedHandler(uint8_t pin);
    void setup();
    void loop();
    void setMode(LedMode mode);

private:
    uint8_t _pin;
    LedMode _currentMode = LedMode::OFF;
    LedMode _previousMode = LedMode::OFF;
    unsigned long _lastToggleTime = 0;
    unsigned long _flickerEndTime = 0;

    void handleBlinking();
};

