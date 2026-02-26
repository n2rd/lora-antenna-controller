# Antenna Configuration Guide

This guide explains how to select between the two supported antenna control types: **RemoteQTH** and **Comtek**.

## Quick Comparison

| Feature | RemoteQTH | Comtek |
|---------|-----------|--------|
| **Directions** | 8 (N, NE, E, SE, S, SW, W, NW) | 4 (N, E, S, W) |
| **Relays** | 6 (3-bit direction + 3 element phase) | 6 (all for quadrant switching) |
| **Coverage** | 45° per sector | 90° per sector |
| **Element Phasing** | Yes - full control | No - quadrant only |
| **Best For** | Serious DX, VHF/UHF | Field portable, casual |
| **Complexity** | Higher | Lower |
| **Cost** | Similar | Similar |

## RemoteQTH (8-Direction)

### Overview

RemoteQTH is a full-featured antenna controller supporting **8 directions** with independent element phasing. Use this for serious DXing operations where you need precise antenna tuning.

### Relay Configuration

```
Direction | Relay 0 | Relay 1 | Relay 2 | Relay 3 | Relay 4 | Relay 5 | Purpose
----------|---------|---------|---------|---------|---------|---------|----------
N (000°)  |    0    |    0    |    0    |    0    |    0    |    0    | North
NE (045°) |    1    |    0    |    0    |    0    |    0    |    0    | Northeast
E (090°)  |    0    |    1    |    0    |    0    |    0    |    0    | East
SE (135°) |    1    |    1    |    0    |    0    |    0    |    0    | Southeast
S (180°)  |    0    |    0    |    1    |    0    |    0    |    0    | South
SW (225°) |    1    |    0    |    1    |    0    |    0    |    0    | Southwest
W (270°)  |    0    |    1    |    1    |    0    |    0    |    0    | West
NW (315°) |    1    |    1    |    1    |    0    |    0    |    0    | Northwest
```

### Angle Parsing

Any azimuth 0-359° is accepted and mapped to nearest sector:

```
Azimuth Range | Direction
--------------|----------
337.5 - 22.5  | N (000°)
22.5 - 67.5   | NE (045°)
67.5 - 112.5  | E (090°)
112.5 - 157.5 | SE (135°)
157.5 - 202.5 | S (180°)
202.5 - 247.5 | SW (225°)
247.5 - 292.5 | W (270°)
292.5 - 337.5 | NW (315°)
```

### Element Phasing (Relays 3-5)

Three additional relays (3, 4, 5) are available for element phase control:

```
These can be used for:
- Director/reflector element lengths
- Phase shifter selection
- Element sequencing
```

### Use Cases

✅ **Use RemoteQTH if you:**
- Want maximum directional resolution
- Have a 3-element or larger array
- Need to optimize element phasing
- Operate 6m, UHF, or microwave bands
- Want to compete in DX contests
- Have space for multiple relay relays

### Build Configuration

```bash
# Build for RemoteQTH (8-direction)
cd phaser
pio run -t upload -e adafruit_feather_m0
```

## Comtek (4-Direction)

### Overview

Comtek is a simplified antenna controller supporting **4 cardinal directions**. Use this for portable or temporary operations where simplicity and cost matter more than coverage.

### Relay Configuration

```
Direction | Relay 0 | Relay 1 | Relay 2 | Relay 3 | Relay 4 | Relay 5 | Purpose
----------|---------|---------|---------|---------|---------|---------|----------
N (000°)  |    0    |    0    |    0    |    0    |    0    |    0    | North
E (090°)  |    1    |    0    |    0    |    0    |    0    |    0    | East
S (180°)  |    0    |    1    |    0    |    0    |    0    |    0    | South
W (270°)  |    1    |    1    |    0    |    0    |    0    |    0    | West
```

### Angle Parsing

Any azimuth 0-359° is accepted and mapped to nearest cardinal:

```
Azimuth Range | Direction
--------------|----------
315 - 45      | N (000°)
45 - 135      | E (090°)
135 - 225     | S (180°)
225 - 315     | W (270°)
```

### Unused Relays (3-5)

Three relays remain unassigned and available for:
- Antenna switching
- Amplifier band selection
- Tower light control
- Future expansion

### Use Cases

✅ **Use Comtek if you:**
- Want a field-portable setup
- Have limited relay availability
- Operate simple 2-element arrays
- Need quick deployment/removal
- Are just getting started with remote control
- Want to minimize wiring and complexity

### Build Configuration

```bash
# Build for Comtek (4-direction)
cd phaser
pio run -t upload -e adafruit_feather_m0_comtek
```

## How to Choose

### Decision Tree

```
Do you want 8 directions?
  |
  +-- YES --> Does your antenna need element control?
              |
              +-- YES --> Use RemoteQTH ✓
              +-- NO  --> Use RemoteQTH (get future headroom) ✓
  |
  +-- NO  --> Do you need simple & portable?
              |
              +-- YES --> Use Comtek ✓
              +-- NO  --> Consider RemoteQTH for flexibility
```

### By Use Case

**HF/VHF DX Operator**: RemoteQTH
- 8 directions needed for DX work
- Array optimization critical
- Permanent/semi-permanent installation

**Field Day / Portable**: Comtek
- Quick deploy/recover
- Limited space at portable site
- Casual operating only

**2m/70cm Local Nets**: Comtek
- 4 cardinal directions sufficient
- Vertical or simple array
- Minimal control needs

**Satellite/Microwave**: RemoteQTH
- Precise positioning essential
- Element phasing important
- Full spectrum of directions

**Multi-Band Array (HF+VHF)**: RemoteQTH
- Complex switching requirements
- Maximum flexibility needed
- Professional installation

## Configuration in Code

### Build-Time Selection

The antenna type is selected at **compile time** via the `ANTENNA_CONFIG` setting in `config.h`:

```cpp
// In phaser/include/config.h

// ANTENNA_CONFIG selection:
#define ANTENNA_CONFIG 1  // 1=RemoteQTH (default), 2=Comtek

// Or override at build time:
// pio run -e adafruit_feather_m0          (RemoteQTH)
// pio run -e adafruit_feather_m0_comtek   (Comtek)
```

### PlatformIO Environments

`platformio.ini` provides two pre-configured build profiles:

```ini
[env:adafruit_feather_m0]
; Default: RemoteQTH configuration
build_flags = -DANTENNA_CONFIG=1

[env:adafruit_feather_m0_comtek]
; Comtek configuration
build_flags = -DANTENNA_CONFIG=2
```

### Automatic Direction Mapping

Once compiled, the phaser **automatically accepts** commands for its configured antenna type:

```
RemoteQTH receives:
  AP1000 → Direction 0 (N)
  AP1045 → Direction 1 (NE)
  AP1090 → Direction 2 (E)
  ...AP1315 → Direction 7 (NW)

Comtek receives same COMMANDS but maps differently:
  AP1000 → Direction 0 (N)
  AP1045 → Direction 0 (N) [rounds to nearest cardinal]
  AP1090 → Direction 1 (E)
  AP1135 → Direction 1 (E) [rounds to nearest cardinal]
  ...AP1315 → Direction 3 (W)
```

## Switching Between Types

To change antenna types:

1. **Edit `config.h`** (optional, for manual selection):
   ```cpp
   #define ANTENNA_CONFIG 2  // Change from 1 to 2
   ```

2. **OR use build environment** (recommended):
   ```bash
   cd phaser
   
   # To switch to Comtek:
   pio run -t upload -e adafruit_feather_m0_comtek
   
   # To switch back to RemoteQTH:
   pio run -t upload -e adafruit_feather_m0
   ```

3. **Recompile and upload** to phaser unit

4. **No changes needed** to controller unit (it adapts automatically)

## Troubleshooting

### "Wrong direction when I send AP1045"

**RemoteQTH**: AP1045 → 45° → Maps to direction 1 (NE) ✓

**Comtek**: AP1045 → 45° → Closer to 0° than 90° → Maps to direction 0 (N)

→ **Solution**: Check which antenna type is compiled on phaser

### "Relays don't click"

**Possible causes:**
1. Wrong antenna type compiled
2. Relay wiring reversed
3. Relay coil voltage insufficient (needs 12V min)
4. GPIO pins assigned to wrong relays

→ **Check**: `config.h` relay pin definitions and `ANTENNA_CONFIG` value

### "I have 6 relays but only some activate"

**RemoteQTH**: Uses relays 0-2 for direction, 3-5 available for other uses

**Comtek**: Uses relays 0-1 for direction, 2-5 available for other uses

→ **Solution**: Review relay assignment table above

## Migration Path

### From HF to Multi-Band

Some operators install HF only initially, then add VHF:

1. **Start with Comtek** on HF (simple 2-element array)
   ```bash
   pio run -t upload -e adafruit_feather_m0_comtek
   ```

2. **Later upgrade to RemoteQTH** when adding VHF array
   ```bash
   pio run -t upload -e adafruit_feather_m0
   ```

3. **Firmware update only** - No hardware changes needed if 6 relays available

## Summary

| Need | Config | Relays | Directions |
|------|--------|--------|-----------|
| Full DX capability | RemoteQTH | 6 | 8 |
| Field portable | Comtek | 6 | 4 |
| Casual local | Comtek | 6(2 used) | 4 |
| Professional | RemoteQTH | 6+ | 8+ |

---

**See Also:**
- [PROTOCOL.md](PROTOCOL.md) - Command format details
- [phaser/README.md](../phaser/README.md) - Antenna setup details
- [HARDWARE.md](HARDWARE.md) - Relay wiring diagrams
