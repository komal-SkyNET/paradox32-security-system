#pragma once

#include <Arduino.h>
#include <functional>

// Define the function signature for the event callback
using ParadoxEventCallback = std::function<void(const String&, const String&)>;

class ParadoxHandler;

extern ParadoxHandler paradoxHandler;

class ParadoxHandler {
public:
    explicit ParadoxHandler(HardwareSerial& serial);
    void setup(ParadoxEventCallback callback);
    void loop();

    // Public methods for controlling the panel
    void setPassword(const char* password);
    bool login();
    void arm(uint8_t partition, uint8_t arm_mode);
    void disarm(uint8_t partition, uint8_t disarm_mode);
    void requestStatus();
    void requestZoneStatus();
    void requestPartitionStatus();
    void disconnect();

private:
    HardwareSerial& _serial;
    ParadoxEventCallback _eventCallback;
    byte _buffer[37];
    bool _panelConnected = false;
    bool _isLoggingIn = false;
    char _password[7];
    unsigned long _lastActivityTime = 0;
    unsigned long _lastPollTime = 0;

    void processBuffer();
    void processZoneStatus();
    void processPartitionStatus();
    void sendCommand(byte* commandData);
    byte calculateChecksum(const byte* data);
    const char* getCommandName(byte command);
    void flushSerialBuffer();
};