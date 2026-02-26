# LoRa Antenna Controller System

Professional-grade LoRa-based remote antenna rotator and phaser control system for amateur radio. This monorepo contains both the shack controller unit and the field antenna phaser unit that communicate via long-range LoRa radio.

## System Overview

This is a complete solution for remotely controlling antenna azimuth rotation and element phasing over long distances (up to several kilometers line-of-sight via LoRa). Designed for amateur radio operators who need reliable antenna control without running cables through walls or across properties.

**Controller Unit** (Shack)
- Feather M0 with LoRa radio, OLED display, GPIO expander
- 8 directional buttons + PTT button for telemetry requests
- Real-time antenna position and telemetry display
- DCU-1 compatible protocol for seamless integration

**Phaser Unit** (Field)
- Feather M0 with LoRa radio and relay switching
- INA3221 power/voltage monitoring
- Reverse power (SWR) measurement via 12-bit ADC
- 6 relay outputs for antenna element control
- Supports RemoteQTH (8-direction) or Comtek (4-direction) configurations

## Quick Start

### Prerequisites

- 2x Adafruit Feather M0 microcontrollers
- 2x RFM95W LoRa radios (915 MHz)
- Controller: SH1106 OLED display, MCP23017 GPIO expander, 8 buttons
- Phaser: 6-relay module, Adafruit INA3221, 12-bit ADC capable MCU
- PlatformIO CLI or IDE installed
- USB cables for programming

### Build & Deploy

#### 1. Clone Repository
```bash
git clone https://github.com/n2rd/lora-antenna-controller.git
cd lora-antenna-controller
```

#### 2. Build and Flash Controller
```bash
cd controller
pio run -t upload
```

#### 3. Build and Flash Phaser (choose antenna type)
```bash
cd ../phaser

# For RemoteQTH (8-direction rotator)
pio run -t upload -e adafruit_feather_m0

# OR for Comtek (4-direction switcher)
pio run -t upload -e adafruit_feather_m0_comtek
```

#### 4. Connect Hardware
- Wire GPIO, relays, display, and sensors as specified in each unit's README
- Power both units (3.3V via USB or battery)
- Place units within LoRa range (open field: 5-15+ km typical)

#### 5. Test
- Press directional buttons on controller
- Verify phaser relays click and antenna moves
- Press PTT button to request telemetry display

## Project Structure

```
lora-antenna-controller/
├── README.md                    # This file - System overview
├── LICENSE                      # MIT License
│
├── controller/                  # Shack control unit
│   ├── README.md                # Controller-specific documentation
│   ├── platformio.ini           # PlatformIO configuration
│   ├── src/main.cpp             # Main controller code
│   ├── include/
│   │   ├── config.h             # Hardware pins, constants
│   │   ├── protocol.h           # Protocol definitions
│   │   └── hardware.h           # Hardware interface declarations
│   ├── lib/                     # Custom libraries (if any)
│   └── test/                    # Unit tests
│
├── phaser/                      # Field antenna phaser unit
│   ├── README.md                # Phaser-specific documentation
│   ├── platformio.ini           # PlatformIO configuration
│   ├── src/main.cpp             # Main phaser code
│   ├── include/
│   │   ├── config.h             # Hardware pins, relay configs
│   │   └── protocol.h           # Protocol definitions
│   ├── lib/                     # Custom libraries
│   └── test/                    # Unit tests
│
├── docs/                        # Shared documentation
│   ├── QUICK_START.md           # Detailed setup guide
│   ├── PROTOCOL.md              # DCU-1 protocol specification
│   ├── HARDWARE.md              # Component list, pinouts, wiring
│   ├── ANTENNA_CONFIG.md        # RemoteQTH vs Comtek guide
│   ├── TROUBLESHOOTING.md       # Common issues and fixes
│   └── REFACTORING_SUMMARY.md   # Code quality improvements
│
└── hardware/                    # Hardware design files (optional)
    ├── controller_schematic.pdf
    └── phaser_schematic.pdf
```

## Features

### Controller Unit
- **8-Direction Control**: N, NE, E, SE, S, SW, W, NW buttons
- **PTT Integration**: Single button for telemetry refresh
- **Real-Time Display**: OLED shows current direction, altitude, voltage, current, SWR
- **Serial Interface**: 4800 baud DCU-1 compatible commands
- **Robust Communication**: RadioHead reliable datagram with automatic retries

### Phaser Unit
- **Flexible Antenna Support**: 
  - RemoteQTH 8-direction with full element phasing
  - Comtek 4-direction quadrant switching
  - Compile-time selection, no code changes needed
- **Comprehensive Telemetry**:
  - Current antenna position (0-7 for RemoteQTH, 0-3 for Comtek)
  - Bus voltage & current via INA3221
  - Reverse power (SWR proxy) via 12-bit ADC
  - MCU supply voltage
  - Signal strength (RSSI)
- **Multiple Command Types**: Direction set, position query, power query

## Protocol

The system uses a modified DCU-1 compatible protocol:

| Command | Format | Response | Purpose |
|---------|--------|----------|---------|
| Set Direction | `AP1XXX` | `;DIRECTION` | Move antenna to bearing XXX (000-359°) |
| Query Position | `AI1` | `;XYZr...v...i...b...` | Get antenna position and telemetry |
| Query Power | `V` | `VPPPPPP` | Get reverse power reading |

Full protocol documentation in [docs/PROTOCOL.md](docs/PROTOCOL.md).

## Antenna Configuration

Both RemoteQTH and Comtek antenna types are supported. Select at compile time via build environment:

```bash
# RemoteQTH (8-direction)
pio run -t upload -e adafruit_feather_m0

# Comtek (4-direction)
pio run -t upload -e adafruit_feather_m0_comtek
```

See [docs/ANTENNA_CONFIG.md](docs/ANTENNA_CONFIG.md) for detailed comparison and selection guide.

## Documentation

- [Controller README](controller/README.md) - Detailed controller operation and pinouts
- [Phaser README](phaser/README.md) - Phaser configuration, relay maps, antenna setup
- [Quick Start Guide](docs/QUICK_START.md) - Step-by-step build and deployment
- [Protocol Specification](docs/PROTOCOL.md) - Complete command/response reference
- [Hardware Guide](docs/HARDWARE.md) - Component list, wiring diagrams, pinouts
- [Antenna Configuration](docs/ANTENNA_CONFIG.md) - RemoteQTH vs Comtek selection
- [Troubleshooting](docs/TROUBLESHOOTING.md) - Common issues and solutions
- [Refactoring Summary](docs/REFACTORING_SUMMARY.md) - Code quality improvements and metrics

## Hardware Requirements

### Controller Unit

| Component | Notes |
|-----------|-------|
| Adafruit Feather M0 | ATSAMD21G18 @48MHz |
| RFM95W LoRa Radio | 915 MHz, SMA antenna |
| SH1106 OLED Display | 1.3" I2C (address 0x3C) |
| MCP23017 GPIO Expander | I2C (address 0x20) for buttons/LEDs |
| 8 Momentary Push Buttons | Connected via MCP23017 |
| 8 LEDs | Optional status indicators |
| 3.3V USB Power | Or 2-cell Li-Ion battery |

### Phaser Unit

| Component | Notes |
|-----------|-------|
| Adafruit Feather M0 | ATSAMD21G18 @48MHz |
| RFM95W LoRa Radio | 915 MHz, SMA antenna |
| 6-Channel Relay Module | 12V coil relays for antenna switching |
| Adafruit INA3221 | 3-channel voltage/current monitor (I2C @0x40) |
| 12-bit ADC | For reverse power measurement |
| 12V Power Supply | For relay coils |
| 3.3V USB Power | For Feather |

## Building & Flashing

This project uses PlatformIO for build management:

```bash
# Install PlatformIO (if not already installed)
pip install platformio

# Build controller
cd controller
pio run

# Flash controller to device (USB must be connected)
pio run -t upload

# Monitor serial output
pio run -t monitor

# For phaser with antenna selection
cd ../phaser
pio run -t upload -e adafruit_feather_m0              # RemoteQTH
# OR
pio run -t upload -e adafruit_feather_m0_comtek       # Comtek
```

## Serial Communication

### Controller Serial
- **Baud Rate**: 4800 (DCU-1 compatible)
- **Format**: 8N1
- **Purpose**: Accepts manual commands, displays telemetry
- **Example**: Send `AP1045` to move antenna to 45° (NE)

### Phaser Serial (Debug)
- **Baud Rate**: 115200
- **Format**: 8N1
- **Purpose**: Debug messages and status information
- **Example**: See relay clicking and sensor readings in real-time

## Contributing

This is a hobby project made available for the amateur radio community. Contributions are welcome!

Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines on:
- Reporting bugs
- Submitting feature requests
- Submitting pull requests
- Code style standards
- Testing requirements

## License

This project is provided under the MIT License. See [LICENSE](LICENSE) file for full terms.

## Support & Contact

For issues, questions, or suggestions:
- Open an issue on GitHub
- Check [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) for common problems
- Review the protocol specification in [docs/PROTOCOL.md](docs/PROTOCOL.md)

## Acknowledgments

- Based on DCU-1 rotator protocol for compatibility
- RadioHead library for reliable LoRa communication
- Adafruit libraries for sensor/display interfaces
- Amateur radio community for inspiration and feedback

## Disclaimer

This project is provided as-is for educational and hobby use. Users are responsible for:
- Following FCC regulations regarding LoRa operation (USA)
- Ensuring antenna installation safety and structural integrity
- Proper grounding and lightning protection
- Testing in local conditions before critical operation

---

**Status**: v1.0.0 - Production Ready
**Last Updated**: February 2026
**GitHub**: https://github.com/n2rd/lora-antenna-controller
