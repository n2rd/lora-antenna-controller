/**
 * @file main.cpp
 * @brief LoRa Antenna Phaser - Remote Antenna Rotator Control
 *
 * Controls antenna rotation relays based on LoRa commands from controller.
 * Provides telemetry including:
 * - Current antenna azimuth position
 * - Bus voltage and current consumption
 * - Reverse power (SWR) measurement via ADC
 * - MCU supply voltage
 * - Signal strength (RSSI)
 *
 * Remote Antenna Configuration:
 * - 8-direction rotator (RemoteQTH)
 * - 6 relay outputs for element switching
 * - INA3221 3-channel current/voltage monitor
 * - ADC for reverse power measurement
 *
 * Hardware:
 * - Adafruit Feather M0
 * - RFM95W LoRa Radio (915 MHz)
 * - 6-channel relay module
 * - Adafruit INA3221 Breakout
 * - 12-bit ADC for SWR measurement
 *
 * @author Your Name
 * @date 2024
 */

// ============================================================================
// INCLUDES
// ============================================================================

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <avr/dtostrf.h>

// Radio libraries
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

// Sensor libraries
#include "Adafruit_INA3221.h"

// Project headers
#include "config.h"
#include "protocol.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

// LoRa radio objects
RH_RF95 rf95(RF95_CS, RF95_INT);
RHReliableDatagram rf95_manager(rf95, MY_ADDRESS);

// Current and voltage monitor
Adafruit_INA3221 ina3221;

// ============================================================================
// APPLICATION STATE
// ============================================================================

/** @brief Current antenna direction (0-7) */
int current_direction = DIR_N;

/** @brief Target direction (for future movement) */
int target_direction = DIR_N;

/** @brief Bus voltage in millivolts */
int bus_voltage_mv = 0;

/** @brief Bus current in milliamps */
int bus_current_ma = 0;

/** @brief Command buffer from controller */
char command_buffer[MAX_COMMAND_LEN];

/** @brief Actual command length received */
int command_length = 0;

/** @brief Reply buffer */
uint8_t reply_buffer[RH_RF95_MAX_MESSAGE_LEN];

/** @brief Reply buffer length */
int reply_length = 0;

/** @brief Packet counter */
int16_t packet_count = 0;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

void init_all_hardware(void);
void set_antenna_direction(int direction);
void measure_sensors(void);
int read_reverse_power_adc(void);
void build_position_reply(int direction);
void build_power_reply(void);
void append_to_reply(const char* str, int len);
void process_command(void);
void handle_set_direction(int direction);
void handle_position_query(void);
void handle_power_query(void);

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * @brief Initialize all hardware subsystems
 *
 * Sets up:
 * - Serial communications
 * - LoRa radio
 * - Relay control pins
 * - INA3221 voltage/current monitor
 * - ADC for reverse power measurement
 *
 * Halts on critical hardware failures.
 */
void init_all_hardware(void) {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n========== LoRa Antenna Phaser Starting ==========");
    
    // Set up relay control pins
    pinMode(RELAY_1, OUTPUT);
    pinMode(RELAY_2, OUTPUT);
    pinMode(RELAY_3, OUTPUT);
    pinMode(RELAY_4, OUTPUT);
    pinMode(RELAY_56, OUTPUT);
    pinMode(RELAY_78, OUTPUT);
    pinMode(LED, OUTPUT);
    
    // Initialize all relays to safe state
    set_antenna_direction(DIR_N);
    Serial.println("✓ Relay outputs configured");
    
    // Initialize LoRa radio
    if (!rf95_manager.init()) {
        Serial.println("ERROR: RF95 radio initialization failed!");
        while (1) {
            digitalWrite(LED, HIGH);
            delay(100);
            digitalWrite(LED, LOW);
            delay(100);
        }
    }
    Serial.println("✓ LoRa Radio initialized");
    
    // Configure radio
    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("ERROR: Failed to set radio frequency!");
        while (1);
    }
    rf95.setTxPower(20, false);
    rf95_manager.setTimeout(1000);
    Serial.printf("✓ Radio configured: %.1f MHz, TX Power 20 dBm\n", RF95_FREQ);
    
    // Initialize INA3221 current/voltage monitor
    if (!ina3221.begin(INA3221_I2C_ADDRESS, &Wire)) {
        Serial.println("ERROR: INA3221 initialization failed!");
        digitalWrite(LED, HIGH);
        while (1);
    }
    ina3221.setAveragingMode(INA3221_AVG_16_SAMPLES);
    ina3221.setShuntResistance(0, 0.10);  // Channel 0: Load
    ina3221.setShuntResistance(1, 0.10);  // Channel 1: 5V supply
    Serial.println("✓ INA3221 Current/Voltage Monitor initialized");
    
    // Initial sensor reading
    measure_sensors();
    
    Serial.println("========== All systems ready ==========\n");
}

// ============================================================================
// RELAY CONTROL
// ============================================================================

/**
 * @brief Set antenna relays for specified direction
 *
 * Configures all 6 relay outputs according to the RemoteQTH
 * antenna configuration table for the given direction.
 *
 * @param direction Direction enum (0-7)
 */
void set_antenna_direction(int direction) {
    if (direction < 0 || direction >= NUM_DIRECTIONS) {
        return;  // Invalid direction
    }
    
    DEBUG_PRINTF("Setting relays for direction %d\n", direction);
    
    // Apply relay configuration
    digitalWrite(RELAY_1, RELAY_POSITIONS[direction][0]);
    digitalWrite(RELAY_2, RELAY_POSITIONS[direction][1]);
    digitalWrite(RELAY_3, RELAY_POSITIONS[direction][2]);
    digitalWrite(RELAY_4, RELAY_POSITIONS[direction][3]);
    digitalWrite(RELAY_56, RELAY_POSITIONS[direction][4]);
    digitalWrite(RELAY_78, RELAY_POSITIONS[direction][5]);
    
    current_direction = direction;
    DEBUG_PRINTF("✓ Antenna direction set to %d (%s°)\n",
                direction, DIRECTION_ANGLES[direction]);
}

// ============================================================================
// SENSOR MEASUREMENTS
// ============================================================================

/**
 * @brief Read bus voltage from INA3221 channel 0
 *
 * @return Bus voltage in millivolts
 */
int read_bus_voltage(void) {
    float voltage = ina3221.getBusVoltage(0);
    return (int)round(1000.0f * voltage);
}

/**
 * @brief Read bus current from INA3221 channel 0
 *
 * @return Current in milliamps
 */
int read_bus_current(void) {
    float current = ina3221.getCurrentAmps(0);
    return (int)round(1000.0f * current);  // Convert A to mA
}

/**
 * @brief Read MCU supply voltage from INA3221 channel 1
 *
 * @return Supply voltage in millivolts
 */
int read_mcu_voltage(void) {
    float voltage = ina3221.getBusVoltage(1);
    return (int)round(1000.0f * voltage);
}

/**
 * @brief Read reverse power ADC and average samples
 *
 * Accumulates ADC_AVG_COUNT samples with small delays between reads.
 *
 * @return Averaged ADC count (0-1023 for 10-bit ADC)
 */
int read_reverse_power_adc(void) {
    long adc_sum = 0;
    
    for (int i = 0; i < ADC_AVG_COUNT; i++) {
        adc_sum += analogRead(REV_POWER_PIN);
        delay(ADC_SAMPLE_DELAY);
    }
    
    int avg = adc_sum / ADC_AVG_COUNT;
    DEBUG_PRINTF("Reverse power ADC average: %d\n", avg);
    return avg;
}

/**
 * @brief Measure all sensors and update global state
 *
 * Reads and stores:
 * - Bus voltage and current
 * - MCU supply voltage
 * - Reverse power ADC reading
 */
void measure_sensors(void) {
    bus_voltage_mv = read_bus_voltage();
    bus_current_ma = read_bus_current();
    
    DEBUG_PRINTF("Bus: %d mV, %d mA\n", bus_voltage_mv, bus_current_ma);
    DEBUG_PRINTF("MCU Supply: %d mV\n", read_mcu_voltage());
}

// ============================================================================
// REPLY BUILDING
// ============================================================================

/**
 * @brief Build a position reply
 *
 * Format: ";XYZrRRRRvVVVVViIIIbBBBB"
 *
 * @param direction Antenna direction (0-7)
 */
void build_position_reply(int direction) {
    reply_length = 0;
    
    // Position prefix and azimuth
    reply_buffer[reply_length++] = REPLY_PREFIX_POS;
    reply_buffer[reply_length++] = DIRECTION_ANGLES[direction][0];
    reply_buffer[reply_length++] = DIRECTION_ANGLES[direction][1];
    reply_buffer[reply_length++] = DIRECTION_ANGLES[direction][2];
    
    // RSSI
    char rssi_str[6];
    sprintf(rssi_str, "r%+04d", rf95.lastRssi());
    for (int i = 0; rssi_str[i] != '\0'; i++) {
        reply_buffer[reply_length++] = rssi_str[i];
    }
    
    // Bus voltage (5 digits, no decimal)
    char volt_str[8];
    sprintf(volt_str, "v%05d", bus_voltage_mv);
    for (int i = 0; volt_str[i] != '\0'; i++) {
        reply_buffer[reply_length++] = volt_str[i];
    }
    
    // Bus current (3 digits, e.g., "i500" = 500mA)
    char curr_str[6];
    sprintf(curr_str, "i%03d", bus_current_ma);
    for (int i = 0; curr_str[i] != '\0'; i++) {
        reply_buffer[reply_length++] = curr_str[i];
    }
    
    // MCU battery voltage
    int mcu_volt = read_mcu_voltage();
    char batt_str[8];
    sprintf(batt_str, "b%04d", mcu_volt);
    for (int i = 0; batt_str[i] != '\0'; i++) {
        reply_buffer[reply_length++] = batt_str[i];
    }
    
    DEBUG_PRINTF("Position reply length: %d\n", reply_length);
}

/**
 * @brief Build a power/telemetry reply
 *
 * Format: "VPPPPPP" where PPPPPP is reverse power in watts
 * Example: "V1500.6" = 1500.6W
 */
void build_power_reply(void) {
    reply_length = 0;
    
    // Read reverse power
    int adc_reading = read_reverse_power_adc();
    
    // Convert ADC to power in watts
    // RemoteQTH calibration factor
    float rev_voltage = adc_reading * REV_POWER_CONVERSION_FACTOR;
    float rev_power = (rev_voltage * rev_voltage) / 100.0f;  // Z0=50Ω formula
    
    // Format power string: "V" + 6 characters
    reply_buffer[reply_length++] = REPLY_PREFIX_PWR;
    
    char power_str[8];
    dtostrf(rev_power, 6, 1, power_str);  // 6 chars total, 1 decimal
    for (int i = 0; power_str[i] != '\0' && reply_length < (int)sizeof(reply_buffer); i++) {
        reply_buffer[reply_length++] = power_str[i];
    }
    
    DEBUG_PRINTF("Power: %.1f W (ADC: %d)\n", rev_power, adc_reading);
    DEBUG_PRINTF("Power reply length: %d\n", reply_length);
}

// ============================================================================
// COMMAND PROCESSING
// ============================================================================

/**
 * @brief Parse command and extract direction from AP1### format
 *
 * Converts azimuth string (000-359) to direction enum (0-7).
 * Maps the second digit to determine direction:
 * - 000/360: N, 045: NE, 090: E, 135: SE
 * - 180: S, 225: SW, 270: W, 315: NW
 *
 * @return Direction enum (0-7), or current_direction if parse error
 */
int parse_direction_from_command(void) {
    if (command_length < 7) {
        return current_direction;  // Invalid command
    }
    
    // Extract digits from command_buffer[3], command_buffer[4], command_buffer[5]
    // which contain the azimuth (e.g., "045" for northeast)
    int digit2 = (int)command_buffer[4] - '0';  // Middle digit
    
    // Map angle to direction
    switch (digit2) {
        case 0: return DIR_N;    // 000 or 360
        case 4: return DIR_NE;   // 045
        case 9: return DIR_E;    // 090
        case 3: return DIR_SE;   // 135
        case 8: return DIR_S;    // 180
        case 2: return DIR_SW;   // 225
        case 7: return DIR_W;    // 270
        case 1: return DIR_NW;   // 315
        case 6: return DIR_N;    // 360
        default: return current_direction;
    }
}

/**
 * @brief Handle direction set command (AP1###)
 *
 * @param direction Direction enum (0-7)
 */
void handle_set_direction(int direction) {
    Serial.printf("Setting target direction to %d (%s°)\n",
                  direction, DIRECTION_ANGLES[direction]);
    
    // Check if terminator is CR (execute now) or semicolon (store only)
    char terminator = command_buffer[command_length - 1];
    
    if (terminator == CMD_TERMINATOR_CR) {
        // Execute immediately
        set_antenna_direction(direction);
    } else {
        // Store as target
        target_direction = direction;
    }
    
    // Measure and respond
    measure_sensors();
    build_position_reply(current_direction);
}

/**
 * @brief Handle position information query (AI1)
 */
void handle_position_query(void) {
    DEBUG_PRINTLN("Position query received");
    measure_sensors();
    build_position_reply(current_direction);
}

/**
 * @brief Handle power/telemetry report request (V)
 */
void handle_power_query(void) {
    DEBUG_PRINTLN("Power report request received");
    measure_sensors();
    build_power_reply();
}

/**
 * @brief Process received command from controller
 *
 * Parses DCU-1 protocol commands and takes appropriate action:
 * - AP1###  or AP1###\r = Set direction
 * - AI1 ; or AM1        = Report position/execute
 * - V                   = Report power/telemetry
 * - ;                   = Stop/emergency stop
 */
void process_command(void) {
    if (command_length == 0) {
        return;  // Empty command
    }
    
    DEBUG_PRINTF("Processing command length %d: %s\n", command_length, command_buffer);
    
    // Single character commands
    if (command_length == 1) {
        switch (command_buffer[0]) {
            case CMD_TYPE_POWER:      // 'V' - Report power
                handle_power_query();
                break;
            case ';':                 // ';' - Stop
                DEBUG_PRINTLN("Stop command");
                measure_sensors();
                build_position_reply(current_direction);
                break;
            default:
                Serial.printf("Unknown single-char command: %c\n", command_buffer[0]);
                break;
        }
        return;
    }
    
    // Multi-character commands (3 or 7 bytes)
    if (command_length == 3) {
        // Format: "?I1" or "?M1" where ? is ignored
        if (command_buffer[1] == CMD_TYPE_INFO_I) {
            handle_position_query();
        } else if (command_buffer[1] == CMD_TYPE_INFO_M) {
            DEBUG_PRINTLN("Movement command - executing stored target direction");
            set_antenna_direction(target_direction);
            measure_sensors();
            build_position_reply(current_direction);
        }
        return;
    }
    
    if (command_length == 7) {
        // Format: AP1###\r  - Set direction
        if (command_buffer[0] == CMD_PREFIX_A && 
            command_buffer[1] == CMD_PREFIX_P &&
            command_buffer[2] == CMD_PREFIX_1) {
            
            int direction = parse_direction_from_command();
            handle_set_direction(direction);
        } else {
            Serial.printf("Malformed set-direction command\n");
        }
        return;
    }
    
    DEBUG_PRINTF("Unexpected command length: %d\n", command_length);
}

// ============================================================================
// MAIN SETUP AND LOOP
// ============================================================================

void setup() {
    init_all_hardware();
}

void loop() {
    // Check if a message is available
    if (rf95_manager.available()) {
        uint8_t len = sizeof(command_buffer);
        uint8_t from;
        
        // Receive message
        if (rf95_manager.recvfromAck((uint8_t*)command_buffer, &len, &from)) {
            
            // Only process messages from controller
            if (from != CTRL_ADDRESS) {
                DEBUG_PRINTF("Message from unknown address %d, ignoring\n", from);
                return;
            }
            
            // Convert buffer to command
            command_length = len;
            if (command_length > MAX_COMMAND_LEN - 1) {
                command_length = MAX_COMMAND_LEN - 1;
            }
            command_buffer[command_length] = '\0';  // Null terminate
            
            packet_count++;
            
            if (DEBUG) {
                Serial.println("================================");
                Serial.printf("Packet #%d from #%d [RSSI:%d]: ",
                             packet_count, from, rf95.lastRssi());
                Serial.println(command_buffer);
            } else {
                Serial.printf("Packet #%d from #%d\n", packet_count, from);
            }
            
            // Process the command
            process_command();
            
            // Send reply
            uint8_t rlen = (uint8_t)reply_length;
            if (DEBUG) {
                Serial.printf("Sending reply, length %d\n", rlen);
            }
            
            if (!rf95_manager.sendtoWait(reply_buffer, rlen, from)) {
                Serial.println("ERROR: Failed to send reply (no ACK)");
                digitalWrite(LED, HIGH);
                delay(50);
                digitalWrite(LED, LOW);
            } else {
                // Blink LED to indicate successful transmission
                digitalWrite(LED, HIGH);
                delay(10);
                digitalWrite(LED, LOW);
            }
        }
    }
    
    // Small delay to reduce CPU usage
    delay(10);
}


