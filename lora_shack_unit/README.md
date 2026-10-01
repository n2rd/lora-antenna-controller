# LoRa Antenna Controller - Shack Unit

Professional-grade LoRa-based antenna azimuth controller for remote antenna selection and telemetry monitoring from the shack.

## Overview

This project implements a **controller unit** that wirelessly controls antenna rotation via LoRa radio. The controller is mounted in the shack and provides:

- **8-direction antenna control** (N, NE, E, SE, S, SW, W, NW)
- **Real-time telemetry display** on SSD1306 OLED screen
- **Button interface** for quick direction selection (via MCP23017 GPIO expander)
- **PTT button** for requesting reverse power / telemetry
- **Serial interface** for computer control
- **Link quality monitoring** with RSSI display
- **Bidirectional LoRa communication** at 915 MHz

The controller works in tandem with the [lora_field_unit](../lora_field_unit) remote unit, which drives the relay switches to select antenna elements.

## Hardware Requirements

### Controller Unit
- **Microcontroller and LoRa Radio**: Heltec WiFi LoRa 32 V3 (ESP32-S3, SX1262)
- **OLED Display**: SSD1306 128×64 pixel display (built-in, on dedicated I2C bus)
- **GPIO Expander**: MCP23017 I2C to digital I/O (for buttons and LEDs)
- **Buttons**: 8 pushbuttons for direction selection (on MCP23017)
- **LEDs**: 8 LEDs for direction indication (on MCP23017)
- **PTT Switch**: Momentary pushbutton (GPIO3 on ESP32)

### Required Libraries
- `jgromes/RadioLib@^7.0.0` - LoRa radio driver (SX1262)
- `adafruit/Adafruit MCP23017 Arduino Library@^2.3.2` - GPIO expander
- `olikraus/U8g2@^2.35.9` - OLED display driver (SSD1306)

## Hardware Design

PCB design files and bill of materials are available in the `hardware/` folder:

- **[Gerber Files](hardware/shack_gerbers/)** - Complete PCB design for manufacturing
  - Ready for submission to PCB manufacturers (JLCPCB, OSHPark, etc.)
  - Includes all layers for 2-layer board construction

- **[Bill of Materials (BOM)](hardware/shack_bom.csv)** - Complete component list
  - CSV format compatible with most electronics suppliers
  - Includes part numbers, quantities, and reference designators
  - Use for ordering components from Digi-Key, Mouser, etc.

3D-printable enclosure files are available in the `STL/` folder:

- **[3D Enclosure Files](STL/)** - Ready-to-print case for the shack controller
  - `Top.stl` - Top enclosure piece
  - `Bottom.stl` - Bottom enclosure piece
  - `Bracket.stl` - Mount bracket for desk/shelf mounting
  - Recommended for SLA or FDM 3D printers with support material
  - Print orientation and parameters depend on your printer

### Assembly and Manufacturing

1. Download gerber files and submit to your PCB manufacturer
2. Review the BOM CSV file and order components
3. Use a reflow oven or soldering iron for assembly
4. Download and 3D print the STL files for the enclosure and bracket
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

### Display and I2C
| Function | Pin | Notes |
|----------|-----|-------|
| OLED SDA | 17 | I2C1 (built-in display) |
| OLED SCL | 18 | I2C1 (built-in display) |
| OLED RST | 21 | Reset pin |
| MCP SDA | 41 | I2C (external GPIO expander) |
| MCP SCL | 42 | I2C (external GPIO expander) |

### Buttons and User Input
| Function | Pin | Notes |
|----------|-----|-------|
| PTT Button | 3 | GPIO3, INPUT_PULLUP, active LOW |
| Direction Buttons | MCP23017 0-7 | 8 buttons on GPIO expander ports A |
| Direction LEDs | MCP23017 8-15 | 8 LEDs on GPIO expander ports B |

### Power and Control
| Function | Pin | Notes |
|----------|-----|-------|
| Status LED | 35 | Onboard LED (LED_BUILTIN) |
| VEXT EN | 36 | Peripheral power control (active LOW) |

## Features

### Protocol
- **DCU-1 compatible** aperture command format
- **Position format**: `AP1###\r` where `###` is azimuth (000-359)
- **PTT format**: Single character `V` requests telemetry
- **Automatic acknowledgment** with RadioHead reliable datagram

### Display Output
Shows real-time antenna telemetry:
```
Rev 12.5 W
RSSI: -095 dBm
Pos: 045
Dir: NE
```

### Serial Interface
- Baud rate: **115200** (USB CDC on ESP32-S3)
- Can accept direction names: `N`, `NE`, `E`, `SE`, `S`, `SW`, `W`, `NW`
- Can accept angle values: `0`, `45`, `90`, `135`, `180`, `225`, `270`, `315`

### User Interactions
1. **Push buttons (0-7)**: Select antenna direction, sends command immediately
2. **PTT button**: Request reverse power telemetry from remote shack
3. **Serial input**: Remote computer control via RS-232 interface

## Configuration

All hardware pins and protocol parameters are defined in `include/config.h`:

```cpp
#define SX1262_FREQ 915.0       // LoRa frequency
#define MY_ADDRESS 211          // This controller's address
#define DEST_ADDRESS 212        // Remote field unit address
#define MCP_I2C_ADDRESS 0x20    // MCP23017 I2C address
#define OLED_I2C_ADDRESS 0x3C   // SSD1306 OLED I2C address
#define PTT_PIN 3               // PTT button input (GPIO3)
```

## Quick Start with Pre-Built Binaries

Pre-compiled binaries are available in the [GitHub Releases](https://github.com/your-org/lora-antenna-controller/releases):

- `lora_shack_unit.bin` - Complete shack controller unit firmware

### Flashing with ESP Web Tools (Recommended - No Installation Required!)

1. **Download the binary** from [Releases](https://github.com/your-org/lora-antenna-controller/releases)

2. **Connect your Heltec device** via USB cable

3. **Open ESP Web Tools** in your browser:
   - Visit: https://esp.huhn.me/
   - Or use: https://web.esphome.io/

4. **Click "Connect"** and select your device's serial port from the popup

5. **Select the binary file**:
   - Click "Choose file" and select `lora_shack_unit.bin`

6. **Click "Program"** and wait for the flash to complete

7. **Done!** The device will reboot automatically

That's it! No command line needed.

## Building and Uploading

### With PlatformIO (Recommended)

```bash
cd lora_shack_unit

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
   - U8g2 (olikraus/U8g2)
   - Adafruit MCP23017 Arduino Library
5. Upload sketch

## Troubleshooting

### No communication with field unit
1. Check LoRa antenna is properly connected to SMA connector
2. Test radio with debug output enabled (`#define DEBUG 1` in config.h)
3. Check address settings in `config.h` match field unit (MY_ADDRESS=211, DEST_ADDRESS=212)
4. Verify frequency matches: 915.0 MHz

### Display not showing anything
1. Check OLED display is properly seated
2. Verify I2C address is 0x3C (SSD1306)
3. Enable debug output to monitor I2C initialization errors

### Buttons not responding
1. Check MCP23017 is properly seated on breadboard or module
2. Verify I2C address matches config (0x20 default)
3. Check button wiring to MCP pins (should be between pin and GND)
4. Verify MCP I2C address via I2C scanner if issues persist

### PTT button not working
1. Verify GPIO3 is not being used elsewhere
2. Check button wiring (pin 3 to GND for activation)
3. Ensure INPUT_PULLUP is enabled
4. Test with serial debug enabled

### Startup Sequence
1. Serial output shows initialization messages
2. Radio configured and tested
3. Display shows "READY!"
4. Direction LED turns on (matches current antenna position)

### Normal Operation
1. Press a direction button to rotate antenna
2. LED blinks to confirm command transmission
3. Display updates with current direction and telemetry
4. Press PTT to request reverse power reading
5. Display shows reverse power reading from remote unit

### Error Handling
- **Radio init failed**: LED blinks continuously
- **No reply from phaser**: "No Reply!" displays on OLED
- **Send failed**: "Send Failed!" displays on OLED

## Protocol Details

### Command Types

| Command | Format | Response | Purpose |
|---------|--------|----------|---------|
| Set direction | AP1###\r | ;XYZr... | Set antenna azimuth and execute |
| Position query | I1; | ;XYZr... | Request current antenna position |
| Power query | V | VPPPPPP | Request reverse power reading |

### Telemetry Data Returned

Position reply includes:
- Antenna azimuth (000-359°)
- Transmit RSSI (signal strength out, dBm)
- Reverse power measurement (W)
- Bus voltage and current
- MCU supply voltage

## References

- [RadioLib Documentation](https://jgromes.github.io/RadioLib/)
- [Yaesu DCU-1 Rotator Protocol](https://www.yaesu.com)
- [Heltec WiFi LoRa 32 V3 Documentation](https://docs.heltec.org/en/node/esp32/dev-board/lora/index.html)
- [Heltec GitHub](https://github.com/Heltec-Aaron-Lee/WiFi_Kit_series)
- [SX1262 LoRa Module Datasheet](https://www.semtech.com/products/wireless-rf/lora-transceivers/sx1262)
- [MCP23017 I/O Expander Datasheet](https://ww1.microchip.com/en-US/product/mcp23017)
- [U8g2 Library](https://github.com/olikraus/u8g2)
- [Adafruit MCP23017 Arduino Library](https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library)

## Authors

- Rajiv Dewan, N2RD rmdewan@gmail.com

## Acknowledgments

- Adafruit Industries for excellent hardware and libraries
- RadioHead Library by Mike McCauley
- Yaesu Corporation for the DCU-1 protocol specification
