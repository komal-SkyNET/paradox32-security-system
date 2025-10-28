#include "LedHandler.h"
#include "Config.h"

#define BLINK_INTERVAL_FAST 250
#define BLINK_INTERVAL_SLOW 1000
#define FLICKER_DURATION 50

LedHandler::LedHandler(uint8_t pin) : _pin(pin) {}

void LedHandler::setup() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    DEBUG_PRINTLN("[LED] Handler initialized.");
}

void LedHandler::setMode(LedMode newMode) {
    if (_currentMode == newMode) return;

    // Don't interrupt a flicker with a new mode unless it's another flicker
    if (_currentMode == LedMode::FLICKER && newMode != LedMode::FLICKER) {
        _previousMode = newMode; // Store the intended mode for after the flicker
        return;
    }

    _previousMode = _currentMode;
    _currentMode = newMode;
    _lastToggleTime = millis(); // Reset timer on mode change

    // DEBUG_PRINTF("[LED] New mode set: %d\n", static_cast<int>(newMode));

    switch (_currentMode) {
        case LedMode::ON:
            digitalWrite(_pin, HIGH);
            break;
        case LedMode::OFF:
            digitalWrite(_pin, LOW);
            break;
        case LedMode::FLICKER:
            digitalWrite(_pin, !digitalRead(_pin)); // Invert current state for flicker effect
            _flickerEndTime = millis() + FLICKER_DURATION;
            break;
        case LedMode::BLINK_SLOW:
        case LedMode::BLINK_FAST:
            // Handled in loop
            break;
    }
}

void LedHandler::loop() {
    if (_currentMode == LedMode::FLICKER) {
        if (millis() >= _flickerEndTime) {
            // Revert to previous mode after flicker is done
            LedMode modeToRestore = _previousMode;
            _currentMode = LedMode::OFF; // Prevent recursive setMode calls
            setMode(modeToRestore);
        }
        return;
    }

    if (_currentMode == LedMode::BLINK_FAST || _currentMode == LedMode::BLINK_SLOW) {
        handleBlinking();
    }
}

void LedHandler::handleBlinking() {
    unsigned long interval = (_currentMode == LedMode::BLINK_FAST) ? BLINK_INTERVAL_FAST : BLINK_INTERVAL_SLOW;
    if (millis() - _lastToggleTime >= interval) {
        digitalWrite(_pin, !digitalRead(_pin));
        _lastToggleTime = millis();
    }
}

