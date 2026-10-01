# LoRa Antenna Controller

A professional-grade, bidirectional LoRa-based antenna controller system for remote antenna selection and telemetry monitoring. This system is designed for amateur radio operators managing multi-element antennas.

## System Overview

The LoRa Antenna Controller consists of two components working together:

### **Shack Unit** (`lora_shack_unit/`)
- Mounted in the shack for local operator control
- Controls antenna direction via 8 pushbuttons
- Displays real-time antenna status and telemetry on OLED screen
- Supports PTT button for requesting power/SWR measurements
- Serial interface for computer-based remote control
- Hardware: Heltec WiFi LoRa 32 V3 (ESP32-S3) + SSD1306 OLED + MCP23017 GPIO expander

### **Field Unit** (`lora_field_unit/`)
- Mounted at antenna site for remote relay control
- Drives 6-channel relay module to switch antenna elements
- Monitors voltage, current, and reverse power with INA219 + ADC
- Reports telemetry back to controller via LoRa
- Supports both RemoteQTH (8-direction) and Comtek (4-direction) antenna configurations
- Hardware: Heltec WiFi LoRa 32 V3 (ESP32-S3) + INA219 power monitor

## Key Features

- **Wireless Communication**: 915 MHz LoRa (SX1262 radio)
- **Protocol**: Yaesu DCU-1 compatible aperture rotator protocol
- **Antenna Configurations**: 8-direction (RemoteQTH) or 4-direction (Comtek) via compile-time selection
- **Telemetry**: Comprehensive monitoring of voltage, current, RSSI, and reverse power
- **Reliability**: RadioLib with automatic packet retry and acknowledgment
- **Power Monitoring**: Real-time voltage/current on bus and MCU rails

## Hardware

Both units use identical Heltec WiFi LoRa 32 V3 boards (ESP32-S3 with SX1262):
- **Microcontroller**: ESP32-S3 (240 MHz dual-core)
- **LoRa Radio**: SX1262 (915 MHz)
- **Power**: USB-C (microcontroller) + external 12-13.8V supply (field unit)

See individual README files in each folder for detailed hardware specifications, pinouts, and configuration options.

## Getting Started

For detailed information, see:
- **Shack Unit**: [lora_shack_unit/README.md](lora_shack_unit/README.md)
- **Field Unit**: [lora_field_unit/README.md](lora_field_unit/README.md)

Both directories contain the complete firmware code, documentation, and build instructions using PlatformIO.
