# LoRa Antenna Phaser

Professional-grade LoRa-based remote antenna rotator control with multi-element switching and comprehensive telemetry monitoring.

## Overview

This project implements the **phaser/remote unit** that controls antenna rotation relays. The unit is mounted at the antenna site and provides:

- **8-direction antenna switching** (N, NE, E, SE, S, SW, W, NW) - RemoteQTH configuration
- **6 independent relay outputs** for element switching
- **Real-time voltage and current monitoring** via INA3221
- **Reverse power (SWR) measurement** via 12-bit ADC
- **Bidirectional LoRa communication** at 915 MHz
- **Complete telemetry reporting** (voltage, current, RSSI, etc.)

The remote unit operates in conjunction with the [lora_cont](../lora_cont_2) controller unit, receiving antenna rotation commands and returning extensive telemetry data.

## Hardware Requirements

### Phaser Unit
- **Microcontroller**: Adafruit Feather M0 (ATSAMD21G18)
- **LoRa Radio**: RFM95W (915 MHz) - Adafruit Feather LoRa Radio
- **Relay Module**: 6-channel relay interface module
- **Voltage/Current Monitor**: Adafruit INA3221 3-channel power monitor
- **ADC Input**: Analog input from reverse power detector (12-bit, 0-3.3V)

### Required Libraries
- `mikem/RadioHead@^1.120` - LoRa radio driver
- `adafruit/Adafruit INA3221 Library@^1.0.1` - Power monitoring
- `rweather/Crypto@^0.4.0` - Optional encryption support

## Pinouts

| Function | Pin | Notes |
|----------|-----|-------|
| SPI Clock | 24 | SAMD21 SPI |
| SPI MOSI | 23 | SAMD21 SPI |
| SPI MISO | 22 | SAMD21 SPI |
| LoRa CS | 8 | Radio chip select |
| LoRa INT | 3 | Radio interrupt |
| Status LED | 13 | Onboard LED |
| Relay 1 | 6 | Element 1 (North) |
| Relay 2 | 5 | Element 2 (South) |
| Relay 3 | 10 | Element 3 (East) |
| Relay 4 | 11 | Element 4 (West) |
| Relay 5/6 | 12 | Parallel relay group |
| Relay 7/8 | 15 | Parallel relay group |
| Rev Power ADC | A2 | Analog reverse power input |
| I2C SDA | 20 | SAMD21 I2C (INA3221) |
| I2C SCL | 21 | SAMD21 I2C (INA3221) |

## Features

### Relay Configuration Tables

#### RemoteQTH - 8-Direction Configuration

| Direction | Angle | Relay Pattern | Purpose |
|-----------|-------|---------------|---------|
| N | 000° | 0,0,0,0,0,0 | All elements parallel |
| NE | 045° | 0,0,1,1,0,1 | Northeast phasing |
| E | 090° | 1,1,1,1,1,1 | All elements series |
| SE | 135° | 0,1,1,0,0,1 | Southeast phasing |
| S | 180° | 0,0,0,0,1,1 | Reverse configuration |
| SW | 225° | 1,1,0,0,0,1 | Southwest phasing |
| W | 270° | 1,1,1,1,0,0 | West switching |
| NW | 315° | 1,0,0,1,0,0 | Northwest phasing |

#### Comtek - 4-Direction Configuration

| Direction | Angle | Relay Pattern | Purpose |
|-----------|-------|---------------|---------|
| N/NE | 000-045° | 0,0,... | Pattern A |
| E/SE | 090-135° | 1,0,... | Pattern B |
| S/SW | 180-225° | 0,1,... | Pattern C |
| W/NW | 270-315° | 1,1,... | Pattern D |

**Note**: Comtek uses only 2 primary relays for 4 directions. Angles within each quadrant are mapped to the closest cardinal direction.

### Choosing Your Antenna Configuration

**Use RemoteQTH if:**
- You need precise 8-direction control
- Your antenna supports element phasing
- You want maximum directional flexibility
- Implementing a modern antenna platform

**Use Comtek if:**
- You have a legacy Comtek antenna rotator
- You only need 4-direction support
- Relay hardware is limited to 2 main controls
- Backward compatibility is important

### Protocol

- **DCU-1 compatible** aperture rotator protocol
- **Position command**: `AP1###\r` where `###` is azimuth (000-359)
- **Position query**: `AI1;` or `AM1` requests current position
- **Power request**: Single `V` character requests reverse power telemetry
- **Auto-acknowledgment** with RadioHead reliable datagram

### Telemetry Data

Each reply packet contains complete antenna and power measurements:

```
+-------+-----+-----+-----+-----+-----+
| Type  | Dir | RSSI | Volt| Curr| Batt|
| (1)   | (3) | (5)  | (5) | (3) | (4) |
+-------+-----+-----+-----+-----+-----+
;       045   r-095 v13800 i0500 b4200
V       1250.6
```

**Position Reply Format**: `;XYZrRRRRvVVVVViIIIbBBBB`
- `;` - Position reply marker
- `XYZ` - 3-digit azimuth (000-359)
- `rRRRR` - RSSI (e.g., r-095 for -95 dBm)
- `vVVVVV` - Bus voltage in mV (e.g., v13800 = 13.8V)
- `iIII` - Bus current in mA (e.g., i0500 = 500mA)
- `bBBBB` - MCU supply voltage in mV

**Power Reply Format**: `VPPPPPP`
- `V` - Power reply marker
- `PPPPPP` - Reverse power in watts (6 chars, e.g., 1250.6W)

## Configuration

All hardware parameters are defined in `include/config.h`:

### Antenna Controller Type Selection

The phaser unit supports **two antenna controller types** selectable at compile time:

#### RemoteQTH (8-Direction) - DEFAULT
- Supports 8 antenna directions: N, NE, E, SE, S, SW, W, NW
- Most flexible configuration
- Recommended for full-featured antenna control

#### Comtek (4-Direction)
- Supports 4 main directions with combined angles
- Simpler relay switching
- Compatible with older Comtek rotators

### Building for Different Antenna Types

**For RemoteQTH (default, 8-direction):**
```bash
pio run -t upload -e adafruit_feather_m0
```

**For Comtek (4-direction):**
```bash
pio run -t upload -e adafruit_feather_m0_comtek
```

### Configuration at Compile Time

The antenna type is controlled via the `ANTENNA_CONFIG` define in `platformio.ini`:

```ini
; RemoteQTH (default)
build_flags = -D ANTENNA_CONFIG=1

; Comtek
build_flags = -D ANTENNA_CONFIG=2
```

Or override directly in `include/config.h`:
```cpp
#define ANTENNA_CONFIG ANTENNA_REMOTEQTH  // or ANTENNA_COMTEK
```

### Hardware Parameters

```cpp
#define RF95_FREQ 915.0         // LoRa frequency
#define MY_ADDRESS 212          // This remote unit's address
#define CTRL_ADDRESS 211        // Controller unit address
#define INA3221_I2C_ADDRESS 0x40 // Power monitor I2C
#define REV_POWER_PIN A2        // Reverse power ADC
#define REV_POWER_CONVERSION 0.5474F // ADC to power conversion

## Building and Uploading

### With PlatformIO (Recommended)

```bash
# Build the project
pio run -e adafruit_feather_m0

# Upload to device
pio run -t upload -e adafruit_feather_m0

# Monitor serial output (115200 baud)
pio device monitor -b 115200
```

### With Arduino IDE

1. Install Adafruit board definitions
2. Select "Adafruit Feather M0" as board
3. Ensure all required libraries are installed
4. Upload sketch

## Installation

### At Antenna Site
1. Mount Feather M0 in weatherproof enclosure
2. Connect LoRa antenna to SMA connector
3. Wire relay module outputs to antenna controller
4. Connect INA3221 power monitor to supply bus
5. Connect reverse power detector to ADC input (A2)
6. Connect relay coils to their respective elements

### Weatherproofing
- Mount PCB in IP67 rated enclosure
- Apply conformal coating to PCB (recommended for outdoor use)
- Use shielded SMA connector for antenna
- Add moisture-absorbing desiccant packet inside enclosure

## Operation

### Startup Sequence
1. Serial output shows initialization messages
2. Radio configured and tested
3. Relays initialized to safe state (North position)
4. Onboard LED flashes to confirm ready state

### Command Processing
1. Controller sends antenna direction command
2. Phaser receives and parses command
3. Relays configured for requested direction
4. Sensors read (voltage, current, power)
5. Reply packet transmitted back to controller
6. LED flashes once per successful transmission

### Telemetry Measurements

**Voltage Monitoring (INA3221)**:
- Channel 0: Main power bus
- Channel 1: MCU supply rail
- Shunt resistors: 0.10Ω per channel

**Current Measurement**:
- Bus current range: ±3.2A (with 0.10Ω shunt)
- Calibration: Via INA3221 library

**Reverse Power ADC**:
- 12-bit resolution (0-1023 counts)
- Input range: 0-3.3V
- Sampling: 10-sample average
- Conversion factor: 0.5474 (RemoteQTH calibrated)

## Error Handling

- **Radio init failed**: Continuous LED blink, halts operation
- **No INA3221**: Log error to serial, continue with zero telemetry
- **Invalid command**: Log warning, return current position
- **Send failed**: Log error, await next command

### Protocol Compliance

Implements **Yaesu DCU-1** rotator protocol for maximum compatibility:

- **Command set**: AP1###, AI1, V, ;
- **ASCII protocol**: Easy integration with existing systems
- **Binary telemetry**: Optional for future enhancement
- **Reliable delivery**: RadioHead datagram with automatic retry
- **Angle parsing**: Auto-maps requested angle to nearest supported direction

#### Angle-to-Direction Mapping

**RemoteQTH** - Direct 45° increments:
- 000° → N, 045° → NE, 090° → E, 135° → SE, etc.

**Comtek** - Quadrant-based (can accept any angle, maps to nearest quadrant):
- 000-045° → N/NE pattern
- 090-135° → E/SE pattern
- 180-225° → S/SW pattern
- 270-315° → W/NW pattern

Both configurations accept the full 0-359° angle range for maximum compatibility with existing controllers.

## Troubleshooting

### No response from phaser
1. Check LoRa antenna connector is secure
2. Verify power supply is connected (LED should indicate operation)
3. Check address settings in `config.h` match controller
4. Monitor serial output for error messages

### Relays not switching
1. Check relay module power supply (typically 5V, 1A+)
2. Verify relay coil winding voltage matches supply
3. Check GPIO pins are correctly wired to relay inputs
4. Test relay operation with DC power supply directly

### Telemetry not updating
1. Check INA3221 I2C connections
2. Verify I2C pullup resistors (4.7kΩ typical)
3. Check address 0x40 is correct for your INA3221
4. Monitor serial output for I2C init errors

### ADC reverse power readings are wrong
1. Check ADC input voltage is 0-3.3V range
2. Verify voltage divider in reverse power detector
3. Adjust `REV_POWER_CONVERSION_FACTOR` in config.h
4. Calibrate against known SWR bridge readings

## License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## References

- [RadioHead Reliable Datagram](http://www.airspayce.com/mikem/arduino/RadioHead/)
- [Yaesu DCU-1 Protocol](https://www.yaesu.com)
- [Adafruit Feather M0 Documentation](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-rfm95-module)
- [RFM95 LoRa Module Datasheet](https://cdn-shop.adafruit.com/product-files/3173/RFM95_97_98_module+Schematic.pdf)
- [INA3221 Power Monitor Datasheet](https://www.ti.com/lit/ds/symlink/ina3221.pdf)

## Authors

- Your Name - Initial work

## Acknowledgments

- Adafruit Industries for excellent hardware and libraries
- Mike McCauley for RadioHead library
- Yaesu Corporation for DCU-1 protocol specification
