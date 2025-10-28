# Release Notes

## v1.0.0 - First Production Release (October 2025)

🎉 **Production Ready** - Out of incubator, this marks the first stable release of the Paradox MQTT / Home Assistant Bridge.

### Overview

After extensive development, testing, and stabilization efforts spanning September-October 2025, this release represents a fully functional, reliable ESP32 bridge/controller to Paradox alarm systems and MQTT/Home Assistant.

### Key Features

✅ **Stable Serial Communication**
- Robust protocol implementation with automatic error recovery
- Handles panel disconnect messages (0x70) gracefully
- Automatic serial buffer resynchronization on errors
- 30-minute keep-alive polling to maintain session

✅ **Bidirectional Control**
- Arm/Disarm/Stay/Sleep commands
- Zone and partition status requests
- Dynamic password handling via MQTT

✅ **Home Assistant Integration**
- Real-time event streaming via MQTT
- Alarm panel entity with PIN protection
- Motion/presence sensors
- Bell status monitoring
- Entry/exit delay indicators
- Push notifications for security events

✅ **User-Friendly Configuration**
- Zero-code captive portal setup
- Persistent WiFi/MQTT credentials
- Factory reset via BOOT button (5-second hold)
- OTA firmware updates

✅ **Operational Reliability**
- Web-based log viewer with chunked transfer
- LED status indicators for connection state
- Event-driven status logging (reduced noise)
- Memory-efficient operation

### What's Been Tested

- **Hardware**: DOIT ESP32 DEVKIT V1
- **Panel**: Paradox SP5500 AUS
- **Runtime**: Multi-day stability confirmed
- **Commands**: Arm, Disarm, Stay, Sleep, Status requests
- **Events**: Zone status, partition status, bell status, entry/exit delays
- **Integration**: Home Assistant MQTT alarm panel and binary sensors

### Breaking Changes from Beta

None - this is the first public opensource release of my project.

### Upgrade Instructions

**From Development Builds:**
1. Flash v1.0.0 via OTA: `pio run -t upload --upload-port paradox-mqtt-bridge.local`
2. No configuration changes required
3. Existing WiFi/MQTT credentials will be preserved

**First-Time Installation:**
Follow the [Installation guide](README.md#installation) in README.md

### Development History

For detailed incubation notes, bug fixes, and the development journey leading to this release, see [NOTES.md](NOTES.md). This document chronicles:
- Initial RX/TX path implementation
- Login sequence discovery
- Serial communication stability fixes
- 0x70 disconnect handling
- Memory optimization for web logs
- Status polling refinements

### Documentation

- **README.md** - Installation, configuration, and usage
- **Deconstructing-events.md** - Complete MQTT event code reference
- **homeassistant/** - Example HA configurations
- **NOTES.md** - Development history and troubleshooting archive

### License

GNU General Public License v3.0 - See [LICENSE](LICENSE) file

---

**Next Release**: See [NOTES.md](NOTES.md) "To Do" section for planned improvements
