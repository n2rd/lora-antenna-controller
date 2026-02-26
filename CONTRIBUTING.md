# Contributing to LoRa Antenna Controller

Thank you for your interest in contributing to this project! This document provides guidelines and instructions for contributing.

## Code of Conduct

Be respectful and constructive in all interactions. We welcome contributors from all backgrounds and experience levels.

## How to Contribute

### Reporting Bugs

**Before submitting a bug report**, please check the existing issues to avoid duplicates.

When reporting a bug, include:

1. **Clear description** of the problem
2. **Steps to reproduce** the issue
3. **Expected behavior** vs actual behavior
4. **Hardware configuration**: Exact microcontroller, radio, sensors, wiring
5. **Software environment**: Arduino IDE version, PlatformIO version, library versions
6. **Error messages or logs**: Full output if available (use `pio run -t monitor`)
7. **Photos** of hardware setup (if relevant)

Example:
```
Title: Controller doesn't respond after 30 minutes

Environment:
- PlatformIO 6.1.0
- Feather M0 with RFM95W
- SH1106 display, MCP23017 expander

Steps to reproduce:
1. Power on both units
2. Send several direction commands
3. Wait 30 minutes without activity
4. Press button - no response

Expected: Phaser responds to command
Actual: No serial output, button press ignored
```

### Suggesting Features

We welcome feature requests, but please:

1. Check existing issues to avoid duplicates
2. Describe the use case clearly
3. Explain how it benefits users
4. Suggest implementation approach (if you have ideas)

Example:
```
Title: Add elevation (pitch) control support

Use case: Some antenna arrays need both azimuth and elevation control
for satellite tracking.

Proposed solution: Extend protocol to support pitch commands similar to
azimuth (AP2###), add 4 additional relays for elevation in phaser.
```

### Submitting Pull Requests

**Fork and Branch**
```bash
git clone https://github.com/YOUR_USERNAME/lora-antenna-controller.git
cd lora-antenna-controller
git checkout -b fix/your-fix-name
```

**Make Changes**
- Follow the code style (see below)
- Add comments for non-obvious code
- Update relevant documentation
- Test thoroughly before submitting

**Commit**
```bash
git commit -m "Brief description of change

More detailed explanation if needed. Reference issues with #123.
"
```

**Push and Create PR**
```bash
git push origin fix/your-fix-name
# Then create PR on GitHub with clear title and description
```

## Code Style Guidelines

### C++ Code

**General**
- Use 4-space indentation (no tabs)
- Maximum line length: 100 characters
- Use meaningful variable names (avoid `a`, `x`, `tmp`)
- Comment non-obvious logic

**Naming Conventions**
```cpp
// Functions: snake_case
void init_all_hardware()
int parse_direction_from_command()

// Variables: snake_case
int bus_voltage_mv;
int relay_position[8];

// Constants: UPPER_SNAKE_CASE
#define MAX_COMMAND_LEN 16
#define MY_ADDRESS 1

// Enums: PascalCase with UPPER_SNAKE_CASE values
enum Direction {
    DIR_N = 0,
    DIR_NE = 1,
    // ...
};
```

**Comments**
```cpp
// Single-line comment for brief explanations
// Multi-line comments use this style

/**
 * @brief Doxygen-style for functions
 * 
 * @param direction Direction value (0-7)
 * @return true if successful
 */
bool set_antenna_direction(int direction);
```

**Spacing**
```cpp
// Good
if (received_bytes > 0) {
    process_command();
}

for (int i = 0; i < 8; i++) {
    // ...
}

// Avoid
if(received_bytes>0){process_command();}
```

### File Organization

Organize code logically:
```cpp
// 1. Includes
#include <Arduino.h>
#include "config.h"

// 2. Defines and constants
#define BUFFER_SIZE 256

// 3. Type definitions (structs, enums)
struct Command { ... };

// 4. Global variables (minimize these!)
int global_counter = 0;

// 5. Function declarations
void process_command(void);

// 6. Function implementations
void process_command(void) { ... }

// 7. Main setup/loop
void setup() { ... }
void loop() { ... }
```

### Header Files

Organize includes in this order:
```cpp
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
// Standard library headers

#include <RH_RF95.h>
#include "Adafruit_INA3221.h"
// Third-party library headers

#include "config.h"
#include "protocol.h"
// Project headers
```

## Testing

Before submitting a PR:

1. **Compile successfully**
   ```bash
   # For controller
   cd controller && pio run
   
   # For phaser (both configurations)
   cd ../phaser
   pio run -e adafruit_feather_m0
   pio run -e adafruit_feather_m0_comtek
   ```

2. **Flash to hardware** and test basic functionality

3. **Check serial output** for errors
   ```bash
   pio run -t monitor
   ```

4. **Test on both projects** if changes affect both

## Documentation

When adding features, update relevant documentation:

1. **Code comments** - Explain why, not just what
2. **Function documentation** - Use Doxygen format for functions
3. **README.md** - Update feature list or usage examples
4. **docs/ files** - Update protocol, hardware, or configuration docs

Example:
```cpp
/**
 * @brief Set antenna direction via relays
 * 
 * Activates appropriate relays to switch antenna elements.
 * Does NOT do bounds checking; caller must validate direction.
 * 
 * @note Relay switching takes ~50ms; use debounce_pin() first
 * @param direction Direction enum (0-7 or 0-3 depending on config)
 */
void set_antenna_direction(int direction);
```

## Pull Request Process

1. **Update documentation** - Especially if changing behavior
2. **Test thoroughly** - On actual hardware if possible
3. **Keep commits clean** - Logical commits, good messages
4. **Reference issues** - Use "Fixes #123" in PR description
5. **Be responsive** - Respond to review comments promptly

## Development Setup

### Prerequisites
```bash
pip install platformio
# Clone your fork and setup
git clone https://github.com/YOUR_USERNAME/lora-antenna-controller.git
cd lora-antenna-controller
```

### Build All Projects
```bash
# Build both controller and phaser
./build_all.sh  # If provided

# Or manually:
cd controller && pio run && cd ..
cd phaser && pio run -e adafruit_feather_m0 && cd ..
```

### Monitor Serial Output
```bash
# From controller/ or phaser/ directory
pio run -t monitor
```

## Areas We're Looking For Help

- **Documentation**: Expand hardware guides, create video tutorials
- **Arduino IDE Support**: Create Arduino IDE compatible packages
- **Additional Antenna Types**: Implement new rotator protocols
- **Web Interface**: Create web dashboard for remote control
- **Mobile App**: Develop Android/iOS app for portable operation
- **Testing**: Write unit tests for protocol parsing
- **Examples**: Create example sketches for common configurations

## Questions?

- Open an issue for clarification
- Check existing issues for Q&A
- Review documentation in `/docs` folder

Thank you for contributing!
