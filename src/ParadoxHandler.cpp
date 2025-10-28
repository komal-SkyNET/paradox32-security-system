#include "ParadoxHandler.h"
#include "Config.h"

ParadoxHandler::ParadoxHandler(HardwareSerial& serial) : _serial(serial) {}

void ParadoxHandler::setup(ParadoxEventCallback callback) {
    _eventCallback = callback;
    DEBUG_PRINTF("[Paradox] Initializing serial port with RX: %d, TX: %d\n", PARADOX_RX_PIN, PARADOX_TX_PIN);
    _serial.begin(PARADOX_BAUD_RATE, SERIAL_8N1, PARADOX_RX_PIN, PARADOX_TX_PIN);
    _lastActivityTime = millis();
    DEBUG_PRINTLN("[Paradox] Handler initialized.");
}

void ParadoxHandler::loop() {
    if (_serial.available() >= 37) {
        _lastActivityTime = millis(); // Reset timer on any incoming data
        
        for (int i = 0; i < 37; i++) {
            _buffer[i] = _serial.read();
        }

        byte startByte = _buffer[0];

        // Check for all known valid message start bytes from the panel
        if ((startByte & 0xF0) == 0xE0) { // Event message
            DEBUG_PRINTLN("[Paradox] Received event message.");
            processBuffer();
        } else if (startByte == 0x10) { // Login success
            _panelConnected = true;
            DEBUG_PRINTLN("[Paradox] Login successful.");
        } else if (startByte == 0x41) { // Acknowledge for Arm
            DEBUG_PRINTF("[Paradox] Received command acknowledgement: 0x%02X\n", startByte);
        } else if (startByte == 0x51) { // Response for Status Request
            if (_buffer[3] == 0x01) {
                processPartitionStatus();
            } else {
                processZoneStatus();
            }
        } else if (startByte == 0x70) { // Disconnect message from the panel
            _panelConnected = false;
            DEBUG_PRINTLN("[Paradox] Received disconnect message from panel (0x70).");
        } else {
            // Invalid start byte, likely out of sync
            DEBUG_PRINTF("[Paradox] Invalid start byte: 0x%02X. Flushing buffer to resync.\n", startByte);
            flushSerialBuffer();
        }
    }

    // Keep-alive polling
    if (millis() - _lastPollTime > 1800000) {
        DEBUG_PRINTLN("[Paradox] Polling for zone and partition status.");
        requestZoneStatus();
        delay(500); // Add a small delay between requests to not flood the panel
        requestPartitionStatus();
        _lastPollTime = millis();
    }
}

void ParadoxHandler::flushSerialBuffer() {
    DEBUG_PRINTLN("[Paradox] Flushing serial buffer.");
    while (_serial.available() > 0) {
        _serial.read();
    }
}

void ParadoxHandler::processBuffer() {
    byte event = _buffer[7];
    byte sub_event = _buffer[8];
    byte partition = _buffer[9];

    if (event == 48 && sub_event == 3 && !_isLoggingIn) {
        _panelConnected = false;
        DEBUG_PRINTLN("[Paradox] Panel logged off.");
    } else if (event == 48 && sub_event == 2) {
        _panelConnected = true;
        DEBUG_PRINTLN("[Paradox] Panel logged on.");
    }

    switch (event) {
        case 2: // Partition status
            switch (sub_event) {
                case 11: // Disarmed
                    _eventCallback("2", "11");
                    break;
                case 12: // Armed Away
                    _eventCallback("2", "12");
                    break;
                case 13: // Entry Delay
                    _eventCallback("2", "13");
                    break;
                case 14: // Exit Delay
                    _eventCallback("2", "14");
                    break;
            }
            break;
        case 6: // Non-reportable events
            switch (sub_event) {
                case 3: // Armed Stay
                    _eventCallback("2", "3");
                    break;
                case 4: // Armed Sleep
                    _eventCallback("2", "4");
                    break;
            }
            break;
    }

    if (_eventCallback) {
        _eventCallback(String(event), String(sub_event));
    }
}

void ParadoxHandler::processPartitionStatus() {
    DEBUG_PRINTLN("[Paradox] Processing partition status response.");

    // Partition 1 Status (byte 17)
    bool p1_alarm = bitRead(_buffer[17], 4);
    bool p1_stay = bitRead(_buffer[17], 2);
    bool p1_sleep = bitRead(_buffer[17], 1);
    bool p1_arm = bitRead(_buffer[17], 0);

    if (p1_alarm) {
        _eventCallback("2", "6"); // Using a generic "triggered" sub-event
    } else if (p1_arm) {
        _eventCallback("2", "12"); // Armed Away
    } else if (p1_stay) {
        _eventCallback("2", "3"); // Armed Stay
    } else if (p1_sleep) {
        _eventCallback("2", "4"); // Armed Sleep
    } else {
        _eventCallback("2", "11"); // Disarmed
    }
}

void ParadoxHandler::processZoneStatus() {
    DEBUG_PRINTLN("[Paradox] Processing zone status response.");

    // Zone status (bytes 19-22 for zones 1-32)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            int zone = (i * 8) + j + 1;
            bool isOpen = bitRead(_buffer[19 + i], j);
            _eventCallback(isOpen ? "1" : "0", String(zone));
        }
    }

    // Bell status (byte 4, bit 0)
    bool bellOn = bitRead(_buffer[4], 0);
    _eventCallback("3", bellOn ? "1" : "0");
}

byte ParadoxHandler::calculateChecksum(const byte* data) {
    int checksum = 0;
    for (int i = 0; i < 36; i++) {
        checksum += data[i];
    }
    return checksum % 256;
}

const char* ParadoxHandler::getCommandName(byte command) {
    switch (command) {
        case 0x00: return "Login-Pass";
        case 0x40: return "Arm/Disarm";
        case 0x50: return "StatusReq";
        case 0x5F: return "Login-Init";
        case 0x70: return "Disconnect";
        default:   return "Unknown";
    }
}

void ParadoxHandler::sendCommand(byte* commandData) {
    commandData[36] = calculateChecksum(commandData);
    DEBUG_PRINTF("[Paradox] Sending command: %s\n", getCommandName(commandData[0]));
    // for (int i = 0; i < 37; i++) {
    //     DEBUG_PRINTF("%02X ", commandData[i]);
    // }
    // DEBUG_PRINTLN("");
    _serial.write(commandData, 37);
    _serial.flush();
    _lastActivityTime = millis(); // Reset keep-alive timer
}

void ParadoxHandler::disconnect() {
    byte data[37] = {0};
    data[0] = 0x70;
    data[2] = 0x05;
    data[33] = 0x01;
    sendCommand(data);
    _panelConnected = false;
    DEBUG_PRINTLN("[Paradox] Sent disconnect command.");
}

void ParadoxHandler::setPassword(const char* password) {
    strncpy(_password, password, sizeof(_password) - 1);
    _password[sizeof(_password) - 1] = '\0';
}

bool ParadoxHandler::login() {
    _isLoggingIn = true;
    DEBUG_PRINTLN("[Paradox] Starting login procedure.");

    // Ensure we start with a clean session
    disconnect();
    delay(250); // Give the panel a moment to process the disconnect

    // Clear any stale data from the serial buffer before we begin
    flushSerialBuffer();

    byte data[37] = {0};
    unsigned long startTime;

    // Step 1: Initiate login
    data[0] = 0x5F;
    data[1] = 0x20;
    data[33] = 0x05;
    sendCommand(data);

    // Wait for the panel's response to the initiation command
    startTime = millis();
    while (millis() - startTime < 1000) { // 1-second timeout
        if (_serial.available() >= 37) {
            for (int i = 0; i < 37; i++) {
                _buffer[i] = _serial.read();
            }
            goto step2; // Response received, proceed to next step
        }
        delay(50);
    }
    DEBUG_PRINTLN("[Paradox] Login failed: No response to login initiation.");
    _isLoggingIn = false;
    return false;

step2:
    // Step 2: Send password
    memset(data, 0, sizeof(data));
    data[0] = 0x00;
    memcpy(&data[4], &_buffer[4], 6); // Copy bytes from panel response
    data[13] = 0x55;

    // Convert password to bytes
    String passStr = String(_password);
    if (passStr.length() >= 4) {
        data[14] = strtoul(passStr.substring(0, 2).c_str(), NULL, 16);
        data[15] = strtoul(passStr.substring(2, 4).c_str(), NULL, 16);
    }
    if (passStr.length() == 6) {
        data[16] = strtoul(passStr.substring(4, 6).c_str(), NULL, 16);
    }

    data[33] = 0x05;
    sendCommand(data);

    // Wait for the final login confirmation (0x10)
    startTime = millis();
    while (millis() - startTime < 2000) { // 2-second timeout
        if (_serial.available() >= 37) {
            for (int i = 0; i < 37; i++) {
                _buffer[i] = _serial.read();
            }
            if (_buffer[0] == 0x10) {
                _panelConnected = true;
                _isLoggingIn = false;
                // Do not print "Login successful" here, let the main loop handle it
                return true;
            }
        }
        delay(50);
    }

    DEBUG_PRINTLN("[Paradox] Login failed: No confirmation received after sending password.");
    _isLoggingIn = false;
    return false;
}

void ParadoxHandler::arm(uint8_t partition, uint8_t arm_mode) {
    if (!_panelConnected) {
        DEBUG_PRINTLN("[Paradox] Not logged in. Attempting to log in before arming.");
        if (!login()) {
            DEBUG_PRINTLN("[Paradox] Re-login failed. Cannot arm.");
            return;
        }
        delay(250); // Give panel a moment to settle after login
    }
    byte data[37] = {0};
    data[0] = 0x40;
    data[2] = arm_mode; // 0x0A for Arm, 0x0B for Stay, 0x0C for Sleep
    data[3] = partition;
    data[33] = 0x05;
    sendCommand(data);
}

void ParadoxHandler::disarm(uint8_t partition, uint8_t disarm_mode) {
    if (!_panelConnected) {
        DEBUG_PRINTLN("[Paradox] Not logged in. Attempting to log in before disarming.");
        if (!login()) {
            DEBUG_PRINTLN("[Paradox] Re-login failed. Cannot disarm.");
            return;
        }
        delay(250); // Give panel a moment to settle after login
    }
    byte data[37] = {0};
    data[0] = 0x40;
    data[2] = disarm_mode;
    data[3] = partition;
    data[33] = 0x01;
    sendCommand(data);
}



void ParadoxHandler::requestStatus() {
    if (!_panelConnected) {
        DEBUG_PRINTLN("[Paradox] Not logged in. Attempting to log in before requesting status.");
        if (!login()) {
            DEBUG_PRINTLN("[Paradox] Re-login failed. Cannot request status.");
            return;
        }
        delay(250); // Give panel a moment to settle after login
    }
    byte data[37] = {0};
    data[0] = 0x50;
    data[1] = 0x00;
    data[2] = 0x80;
    data[3] = 0x01; // Status type 1
    data[33] = 0x05;
    sendCommand(data);
}

void ParadoxHandler::requestPartitionStatus() {
    if (!_panelConnected) {
        DEBUG_PRINTLN("[Paradox] Not logged in. Cannot request partition status.");
        if (!login()) {
            DEBUG_PRINTLN("[Paradox] Re-login failed. Cannot request partition status.");
            return;
        }
        delay(250); // Give panel a moment to settle after login
    }
    byte data[37] = {0};
    data[0] = 0x50;
    data[1] = 0x00;
    data[2] = 0x80;
    data[3] = 0x01; // Status type 1 for partitions
    data[33] = 0x05;
    sendCommand(data);
}

void ParadoxHandler::requestZoneStatus() {
    if (!_panelConnected) {
        DEBUG_PRINTLN("[Paradox] Not logged in. Cannot request zone status.");
        if (!login()) {
            DEBUG_PRINTLN("[Paradox] Re-login failed. Cannot request zone status.");
            return;
        }
        delay(250); // Give panel a moment to settle after login
    }
    byte data[37] = {0};
    data[0] = 0x50;
    data[1] = 0x00;
    data[2] = 0x80;
    data[3] = 0x00; // Status type 0 for zones
    data[33] = 0x01;
    sendCommand(data);
}