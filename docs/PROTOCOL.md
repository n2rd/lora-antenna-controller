# DCU-1 Protocol Specification

This document describes the command and response protocol used by the LoRa Antenna Controller System. The protocol is designed for compatibility with DCU-1 rotator controllers while adapted for LoRa communication.

## Overview

The system uses a **master-slave model**:
- **Master**: Shack controller unit (initiates commands)
- **Slave**: Field phaser unit (responds to commands)

Communication occurs via LoRa at 915 MHz with RadioHead's reliable datagram layer providing automatic acknowledgment and retry.

## Protocol Basics

### Message Types

1. **Commands** - Sent by controller to phaser (1-7 bytes)
2. **Responses** - Sent by phaser back to controller (variable length)

### Timeout & Retries

- **Command timeout**: 2 seconds
- **Maximum retries**: 3 automatic retries (RadioHead)
- **No manual ACKs needed**: RadioHead handles reliability layer

## Commands

### 1. Set Direction (AP1XXX)

Move antenna to specified azimuth bearing.

```
Format: AP1XXX
  A = 0x41 ('A')
  P = 0x50 ('P')
  1 = 0x31 ('1')
  XXX = 000-359 degrees (ASCII digits)

Total: 7 bytes
Example: AP1045 (move to 45° NE)
         AP1270 (move to 270° W)
```

**Response Format**:
```
;D

  D = direction digit
      RemoteQTH: 0-7 represent N, NE, E, SE, S, SW, W, NW
      Comtek: 0-3 represent N, E, S, W

Example Response: ;2 (East)
```

**Angle Mapping** (Both antenna types accept 0-359°):

RemoteQTH (8-direction):
- 0° = N (sector 0, 337.5-22.5°)
- 45° = NE (sector 1, 22.5-67.5°)
- 90° = E (sector 2, 67.5-112.5°)
- 135° = SE (sector 3, 112.5-157.5°)
- 180° = S (sector 4, 157.5-202.5°)
- 225° = SW (sector 5, 202.5-247.5°)
- 270° = W (sector 6, 247.5-292.5°)
- 315° = NW (sector 7, 292.5-337.5°)

Comtek (4-direction):
- 0° = N (sector 0, 315-45°)
- 90° = E (sector 1, 45-135°)
- 180° = S (sector 2, 135-225°)
- 270° = W (sector 3, 225-315°)

**Error Handling**:
```
;E = Error (invalid direction)
;T = Timeout (command not processed)
```

### 2. Query Position (AI1)

Request current antenna position and telemetry.

```
Format: AI1
  A = 0x41 ('A')
  I = 0x49 ('I')
  1 = 0x31 ('1')

Total: 3 bytes
```

**Response Format**:
```
;DRSSI_V_I_B

Breakdown:
  ; = Start marker (0x3B)
  D = Direction (single digit, see Set Direction mapping)
  R = RSSI high digit (signal strength)
  S = RSSI low digit
  S = Extra digit
  I = Extra digit
  _ = Reserved
  V = Voltage reading
  _ = Separator
  I = Current reading
  _ = Separator
  B = Bus voltage

Full telemetry response:
;X<rssi><angle><voltage><current><mcu_voltage>

Example: ;0-105+12.5+1.2+3.3
  Direction: 0 (North)
  RSSI: -105 dBm
  Altitude: 45° (not used in current implementation)
  Bus voltage: 12.5V
  Bus current: 1.2A
  MCU voltage: 3.3V
```

**Detailed Response**:
For RemoteQTH/Comtek phaser units:
```
;D<RSSI><angle>v<voltage>i<current>b<mcu_v>

D = direction (0-7 or 0-3)
RSSI = signal strength (-165 to -70 dBm)
angle = altitude angle in degrees (usually reserved/0)
voltage = supply voltage in format "+XX.X" (12.5V typical)
current = current draw in format "+X.X" (1.2A typical)
mcu_v = MCU supply in format "+3.X" (3.3V typical)
```

### 3. Query Power (V)

Request reverse power (SWR proxy) reading.

```
Format: V
  V = 0x56 ('V')

Total: 1 byte
```

**Response Format**:
```
VPPPPPP

P = 6 digit ASCII power reading
    Range: 000000-999999 (ADC counts)
    
Example: V001234 (ADC reading of 1234)
```

**Cross-Reference**:
Power reading interpretation depends on antenna load impedance and directional coupler characteristics. Requires calibration per installation.

## Message Reliability

### RadioHead Datagram Layer

The system uses `RHReliableDatagram` which adds:

- **Automatic ACKs**: Layer 4 acknowledgment (transparent to protocol)
- **Auto-retry**: Up to 3 retries on timeout
- **Duplicate filtering**: Prevents duplicate processing

**From Application Perspective:**
```
Flow:
1. Controller sends "AP1045" (7 bytes)
2. RadioHead waits for ACK
3. If no ACK within 2 seconds, retry (up to 3 times)
4. If all retries fail, return timeout error to user

Note: Application sees only success/timeout, not ACK details
```

### Typical Exchange

**Successful Command**:
```
Controller                          Phaser
    |                                 |
    |------ AP1045 ------->|         (receive, validate)
    |                       |---> (process, set relays)
    |                      |------- ;2 ------->|
    |<----- ACK ----------|         (received)
    |                                 |
```

**With Timeout & Retry**:
```
Controller                          Phaser
    |                                 |
    |------ AP1045 ------->|         (no reception)
    |                       |
    | (wait 2 sec, no ACK)
    |------ AP1045 ------->| (retry) |
    |                      |------  ;2 ----->|
    |<----- ACK ----------|
```

## Data Types

### Direction Encoding

**RemoteQTH (8-direction)**
```
Value | Direction | Degrees | Relays
------|-----------|---------|--------
  0   |     N     |    0°   | Config 0
  1   |    NE     |   45°   | Config 1
  2   |     E     |   90°   | Config 2
  3   |    SE     |  135°   | Config 3
  4   |     S     |  180°   | Config 4
  5   |    SW     |  225°   | Config 5
  6   |     W     |  270°   | Config 6
  7   |    NW     |  315°   | Config 7
```

**Comtek (4-direction)**
```
Value | Direction | Degrees | Relays
------|-----------|---------|--------
  0   |     N     |    0°   | Config 0
  1   |     E     |   90°   | Config 1
  2   |     S     |  180°   | Config 2
  3   |     W     |  270°   | Config 3
```

### RSSI (Received Signal Strength Indication)

Signal strength in dBm:
```
-70 dBm   = Very strong (excellent reception)
-100 dBm  = Good (typical open field)
-120 dBm  = Weak (limit of reliable operation)
-140 dBm  = Very weak (frequent retries)
```

## Practical Examples

### Example 1: Move to Northeast

```
User presses NE button on controller

1. Controller sends: "AP1045"
2. Phaser receives and extracts angle 045
3. Phaser maps to direction 1 (NE)
4. Phaser activates relay pattern for direction 1
5. Phaser replies: ";1"
6. Controller receives: ";1"
7. Controller updates display: "NE"
```

### Example 2: Request Telemetry

```
User presses PTT button on controller

1. Controller sends: "AI1"
2. Phaser receives telemetry request
3. Phaser reads sensors:
   - Current direction: 2 (E)
   - Bus voltage: 12.5V
   - Bus current: 1.2A
   - MCU voltage: 3.3V
   - RSSI: -105 dBm
4. Phaser formats and replies: ";2-105 v+12.5i+1.2b+3.3"
5. Controller receives and parses
6. Controller displays on OLED:
   Direction: E
   Voltage: 12.5V
   Current: 1.2A
```

### Example 3: Query Power

```
Controller sends: "V"
Phaser reads ADC: value 1234 counts
Phaser replies: "V001234"
Controller receives and displays SWR indicator
```

## Error Cases

### Invalid Command
```
Send: AP1XXX where XXX > 359
Phaser rejects as out-of-range
Response: ;E (error state)
```

### Communication Loss
```
Send: AP1045
Wait 2 seconds with no ACK
Retry up to 3 times
After 3 retries: Return TIMEOUT error to user
```

### Malformed Response
```
Phaser sends garbled data
RadioHead CRC check fails
Message discarded
Controller times out and retries
```

## Protocol Extensions (Future)

Potential additions while maintaining compatibility:

- **Elevation Control**: AP2### for pitch (elevation)
- **Speed Control**: AP1### with rate limiter
- **Position Feedback**: Extended telemetry with actual azimuth reading
- **Stasis Control**: AP3### for element matching
- **Configuration Queries**: AI2, AI3 for feature discovery

## Compatibility Notes

- **DCU-1 Interface**: Input compatible (AP1###, AI1, V commands)
- **Serial Speed**: Controller uses 4800 baud (DCU-1 standard), phaser uses 115200 for debug
- **Protocol Variant**: Responses include extended telemetry beyond standard DCU-1
- **Antenna Types**: Supports both RemoteQTH and Comtek decodings

## Summary Table

| Command | Bytes | Function | Response |
|---------|-------|----------|----------|
| AP1XXX | 7 | Set azimuth | ;D or ;E |
| AI1 | 3 | Query position | ;D<rssi>... |
| V | 1 | Query power | VPPPPPP |

---

**Protocol Version**: 1.0  
**Last Updated**: February 2026  
**Compatibility**: DCU-1 rotator controllers and derivatives
