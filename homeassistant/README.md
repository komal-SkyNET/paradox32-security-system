# Home Assistant Configuration Files

This directory contains the YAML configuration files for integrating the Paradox MQTT bridge with Home Assistant.

## Setup Instructions

To integrate this into your Home Assistant, you need to tell your main `configuration.yaml` file to include the files from this directory.

Add the following lines to your `configuration.yaml` file:

```yaml
# In configuration.yaml
mqtt: !include homeassistant/mqtt.yaml
template: !include homeassistant/template.yaml
```
