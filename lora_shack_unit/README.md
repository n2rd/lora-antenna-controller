# LoRa Antenna Controller

Professional-grade LoRa-based antenna azimuth controller for remote antenna selection and telemetry monitoring from the shack.

## Overview

This project implements a **controller unit** that wirelessly controls antenna rotation via LoRa radio. The controller is mounted in the shack and provides:

- **8-direction antenna control** (N, NE, E, SE, S, SW, W, NW)
- **Real-time telemetry display** on 1.3" OLED screen
- **Button interface** for quick direction selection  
- **PTT button** for requesting reverse power / telemetry
- **Serial interface** for computer control
- **Link quality monitoring** with RSSI display
- **Bidirectional LoRa communication** at 915 MHz

The controller works in tandem with the [lora_phaser](../lora_phaser_2) remote unit, which drives the relay switches to select antenna elements.

## Hardware Requirements

### Controller Unit
- **Microcontroller**: Adafruit Feather M0 (ATSAMD21G18)
- **LoRa Radio**: RFM95W (915 MHz) - Adafruit Feather LoRa Radio
- **Display**: SH1106 1.3" OLED (128x64 pixels) I2C
- **GPIO Expander**: MCP23017 I2C to digital I/O breakout
- **Buttons**: 8 pushbuttons (1 per direction) on MCP23017 ports A
- **LEDs**: 8 LEDs (1 per direction) on MCP23017 ports B
- **PTT Switch**: Momentary pushbutton on pin 11

### Required Libraries
- `epsilonrt/RadioHead@^1.122.1` - LoRa radio driver
- `adafruit/Adafruit MCP23017 Arduino Library@^2.3.2` - GPIO expander
- `adafruit/Adafruit GFX Library@^1.11.11` - Graphics library
- `adafruit/Adafruit SH110X@^2.1.11` - OLED display driver

## Pinouts

| Function | Pin | Notes |
|----------|-----|-------|
| SPI Clock | 24 | SAMD21 SPI |
| SPI MOSI | 23 | SAMD21 SPI |
| SPI MISO | 22 | SAMD21 SPI |
| LoRa CS | 8 | Radio chip select |
| LoRa INT | 3 | Radio interrupt |
| LoRa IRQ | 3 | Same as INT |
| Status LED | 13 | Onboard LED |
| PTT Button | 11 | Input (pull-up) |
| I2C SDA | 20 | SAMD21 I2C (display, MCP) |
| I2C SCL | 21 | SAMD21 I2C (display, MCP) |

## Features

### Protocol
- **DCU-1 compatible** aperture command format
- **Position format**: `AP1###\r` where `###` is azimuth (000-359)
- **PTT format**: Single character `V` requests telemetry
- **Automatic acknowledgment** with RadioHead reliable datagram

### Display Output
Shows real-time antenna telemetry:
```
Rev 1250.5 W
RSSI: -095 dBm
Pos: 045
Dir: NE
```

### Serial Interface
- Baud rate: **4800** (compatible with DCU-1 controllers)
- Can accept direction names: `N`, `NE`, `E`, `SE`, `S`, `SW`, `W`, `NW`
- Can accept angle values: `0`, `45`, `90`, `135`, `180`, `225`, `270`, `315`

### User Interactions
1. **Push buttons (0-7)**: Select antenna direction, sends command immediately
2. **PTT button**: Request reverse power telemetry from remote shack
3. **Serial input**: Remote computer control via RS-232 interface

## Configuration

All hardware pins and protocol parameters are defined in `include/config.h`:

```cpp
#define RF95_FREQ 915.0         // LoRa frequency
#define MY_ADDRESS 211          // This controller's address
#define DEST_ADDRESS 212        // Remote phaser unit address
#define RF95_CS 8               // Radio chip select
#define RF95_INT 3              // Radio interrupt pin
#define PTT_PIN 11              // PTT button input
```

## Building and Uploading

### With PlatformIO (Recommended)

```bash
# Build the project
pio run -e adafruit_feather_m0

# Upload to device
pio run -t upload -e adafruit_feather_m0

# Monitor serial output
pio device monitor -b 4800
```

### With Arduino IDE

1. Install Adafruit board definitions
2. Select "Adafruit Feather M0" as board
3. Ensure all required libraries are installed
4. Upload sketch

## PCB and Wiring Diagram

Refer to the hardware documentation:
- [Controller Schematic](docs/controller_schematic.png)
- [Relay Wiring Guide](docs/relay_wiring.md)
- [I2C Bus Connections](docs/i2c_layout.md)

## Operation

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

## Troubleshooting

### No communication with phaser unit
1. Check LoRa radio is properly soldered
2. Verify antenna connector is secure
3. Test radio with debug output enabled (`#define DEBUG 1`)
4. Check address settings in `config.h`

### Display not showing anything
1. Check I2C pullup resistors (typically 4.7kΩ)
2. Verify display address (0x3C default)
3. Check SDA/SCL wiring
4. Enable debug output to monitor I2C errors

### Buttons not responding
1. Check MCP23017 is properly inserted in breadboard
2. Verify I2C address matches config (0x20 default)
3. Check button wiring - should be between pin and GND
4. Verify pull-up resistors on MCP pins are enabled

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

- [RadioHead Reliable Datagram Documentation](http://www.airspayce.com/mikem/arduino/RadioHead/)
- [Yaesu DCU-1 Rotator Protocol](https://www.yaesu.com)
- [Adafruit Feather M0 Pinout](https://learn.adafruit.com/adafruit-feather-m0-radio-with-lora-rfm95-module)
- [RFM95 LoRa Module Datasheet](https://cdn-shop.adafruit.com/product-files/3173/RFM95_97_98_module+Schematic.pdf)

## Authors

- Your Name - Initial work

## Acknowledgments

- Adafruit Industries for excellent hardware and libraries
- RadioHead Library by Mike McCauley
- Yaesu Corporation for the DCU-1 protocol specification
