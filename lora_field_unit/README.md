# LoRa Field Unit for Antenna Control

Professional-grade LoRa-based remote antenna rotator control with multi-element switching and comprehensive telemetry monitoring.

## Overview

This project implements the **Field Unit** that controls antenna rotation relays in a RemoteQTH or Comtek 4SQ antenna phasing unit. The field unit is mounted at the antenna site and provides:

- **8-direction antenna switching** (N, NE, E, SE, S, SW, W, NW) - RemoteQTH configuration
- **4-direction antenna switching** (N, E, S, W) - Comtek configuration (via compile-time selection)
- **6 independent relay outputs** for element switching
- **Real-time voltage and current monitoring** via INA219
- **Reverse power (SWR) measurement** via 12-bit ADC
- **Bidirectional LoRa communication** at 915 MHz
- **Complete telemetry reporting** (voltage, current, RSSI, etc.)

The remote unit operates in conjunction with the [lora_shack_unit](../lora_shack_unit) controller unit, receiving antenna rotation commands and returning extensive telemetry data.

## Hardware Requirements

### Field Unit
- **Microcontroller and LoRa Radio**: Heltec WiFi LoRa 32 V4 (ESP32-S3, SX1262)
- **Relay Module**: 6-channel relay interface module (GPIO controlled)
- **Voltage/Current Monitor**: Adafruit INA219 (I2C, address 0x40)
- **ADC Input**: Analog input from reverse power detector (12-bit, 0-3.3V on GPIO6)

### Required Libraries
- `jgromes/RadioLib@^7.0.0` - LoRa radio driver (SX1262)
- `adafruit/Adafruit INA219@^1.2.3` - Power monitoring

## Hardware Design

PCB design files and bill of materials are available in the `hardware/` folder:

- **[Gerber Files](hardware/field_gerbers/)** - Complete PCB design for manufacturing
  - Ready for submission to PCB manufacturers (JLCPCB, OSHPark, etc.)
  - Includes all layers for 2-layer board construction

- **[Bill of Materials (BOM)](hardware/field_bom.csv)** - Complete component list
  - CSV format compatible with most electronics suppliers
  - Includes part numbers, quantities, and reference designators
  - Use for ordering components from Digi-Key, Mouser, etc.

3D-printable enclosure files are available in the `STL/` folder:

- **[3D Enclosure Files](STL/)** - Ready-to-print case for the field unit
  - `Top.stl` - Top enclosure piece
  - `Bottom.stl` - Bottom enclosure piece
  - Recommended for SLA or FDM 3D printers with support material
  - Print orientation and parameters depend on your printer

### Assembly and Manufacturing

1. Download gerber files and submit to your PCB manufacturer
2. Review the BOM CSV file and order components
3. Use a reflow oven or soldering iron for assembly
4. Download and 3D print the STL files for the enclosure
5. See individual component datasheets for proper solder joint techniques

## Pinouts

### LoRa Radio (SX1262 via SPI)
| Function | Pin | Notes |
|----------|-----|-------|
| Radio CS | 8 | Chip Select |
| Radio BUSY | 13 | Busy signal |
| Radio RST | 12 | Reset |
| Radio DIO1 | 14 | Interrupt (IRQ) |
| RF FEM EN | 2 | RF Front-End Module Enable |
| RF FEM PA | 46 | RF Power Amplifier Enable |
| SPI SCLK | 12 | (Hardware SPI) |
| SPI MOSI | 11 | (Hardware SPI) |
| SPI MISO | 13 | (Hardware SPI) |

### Relay Outputs
| Relay | Pin | Purpose |
|-------|-----|---------|
| Relay 1 | 3 | Element 1 |
| Relay 2 | 4 | Element 2 |
| Relay 3 | 5 | Element 3 |
| Relay 4 | 33 | Element 4 |
| Relay 5/6 | 47 | Parallel relay group |
| Relay 7/8 | 48 | Parallel relay group |

### Sensors
| Function | Pin | Notes |
|----------|-----|-------|
| Rev Power ADC | 6 | Analog reverse power input (ADC1 Ch6) |
| I2C SDA | 41 | I2C for INA219 |
| I2C SCL | 42 | I2C for INA219 |
| Status LED | 35 | Onboard LED (LED_BUILTIN) |
| VEXT EN | 36 | Peripheral power control (active LOW) |

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


## Quick Start with Pre-Built Binaries

Pre-compiled binaries are available in the [GitHub Releases](https://github.com/your-org/lora-antenna-controller/releases) for both RemoteQTH and Comtek configurations:

- `lora_field_unit_remoteqth.bin` - 8-direction RemoteQTH antenna controller (default)
- `lora_field_unit_comtek.bin` - 4-direction Comtek antenna controller

### Flashing with ESP Web Tools (Recommended - No Installation Required!)

1. **Download the binary** from [Releases](https://github.com/your-org/lora-antenna-controller/releases)

2. **Connect your Heltec device** via USB cable

3. **Open ESP Web Tools** in your browser:
   - Visit: https://esp.huhn.me/
   - Or use: https://web.esphome.io/

4. **Click "Connect"** and select your device's serial port from the popup

5. **Select the binary file**:
   - Click "Choose file" and select `lora_field_unit_remoteqth.bin` or `lora_field_unit_comtek.bin`

6. **Click "Program"** and wait for the flash to complete

7. **Done!** The device will reboot automatically

That's it! No command line needed.

### Building for Different Antenna Types using PlatformIO

**For RemoteQTH (default, 8-direction):**
```bash
cd lora_field_unit
pio run -t upload -e heltec_wifi_lora_32_V3
```

**For Comtek (4-direction):**
```bash
cd lora_field_unit
pio run -t upload -e heltec_wifi_lora_32_V3 -D ANTENNA_CONFIG=2
```

### Configuration at Compile Time

The antenna type is controlled via the `ANTENNA_CONFIG` define in `include/config.h`:

```cpp
#define ANTENNA_CONFIG ANTENNA_REMOTEQTH  // 1 = RemoteQTH
#define ANTENNA_CONFIG ANTENNA_COMTEK     // 2 = Comtek
```

Or via `platformio.ini`:

```ini
; RemoteQTH (default)
build_flags = -D ANTENNA_CONFIG=1

; Comtek
build_flags = -D ANTENNA_CONFIG=2
```

### Radio and Hardware Parameters

```cpp
#define SX1262_FREQ 915.0         // LoRa frequency
#define MY_ADDRESS 212            // This remote unit's address
#define CTRL_ADDRESS 211          // Controller unit address
#define INA219_I2C_ADDRESS 0x40   // Power monitor I2C address
#define REV_POWER_PIN 6           // Reverse power ADC pin
#define REV_POWER_CONVERSION_FACTOR 371.71F // ADC to power conversion
```


## Building and Uploading

### With PlatformIO

```bash
cd lora_field_unit

# Build the project
pio run -e heltec_wifi_lora_32_V3

# Upload to device
pio run -t upload -e heltec_wifi_lora_32_V3

# Monitor serial output (115200 baud)
pio device monitor -b 115200
```

### With Arduino IDE

1. Install ESP32 board definitions (via Boards Manager: "esp32")
2. Install Heltec board support (https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series)
3. Select "Heltec WiFi LoRa 32(V3)" as board
4. Install required libraries:
   - RadioLib (jgromes/RadioLib)
   - Adafruit INA219
5. Upload sketch

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

**Voltage Monitoring (INA219)**:
- Bus voltage measurement
- MCU supply rail monitoring
- Shunt resistor: 0.10Ω

**Current Measurement**:
- Bus current range: ±3.2A (with 0.10Ω shunt)
- Calibration: Via INA219 library

**Reverse Power ADC**:
- 12-bit resolution (0-1023 counts via analogReadMilliVolts())
- Input range: 0-3.3V
- Sampling: 10-sample average with 3-top averaging
- Conversion factor: 371.71 (calibrated for detector voltage)

## Error Handling

- **Radio init failed**: Continuous LED blink, halts operation
- **No INA219**: Log error to serial, continue with zero telemetry
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
1. Check relay module power supply (typically 12V, 1A+)
2. Verify relay coil winding voltage matches supply
3. Check GPIO pins are correctly wired to relay inputs
4. Test relay operation with DC power supply directly

### Telemetry not updating
1. Check INA219 I2C connections
3. Check address 0x40 is correct for your INA219
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

- [RadioLib Documentation](https://jgromes.github.io/RadioLib/)
- [Yaesu DCU-1 Protocol](https://www.yaesu.com)
- [Heltec WiFi LoRa 32 V3 Documentation](https://docs.heltec.org/en/node/esp32/dev-board/lora/index.html)
- [Heltec GitHub](https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series)
- [SX1262 LoRa Module Datasheet](https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262)
- [Adafruit INA219 Library](https://github.com/adafruit/Adafruit_INA219)
- [INA219 Power Monitor Datasheet](https://www.ti.com/lit/ds/symlink/ina219.pdf)

## Authors

- Rajiv Dewan, N2RD. rmdewan@gmail.com

## Acknowledgments

- Adafruit Industries for excellent hardware and libraries
- Mike McCauley for RadioHead library
- Hygain for DCU-1 protocol specification
