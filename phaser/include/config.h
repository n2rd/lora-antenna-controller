/**
 * @file config.h
 * @brief Configuration and hardware definitions for LoRa Antenna Phaser
 *
 * This file contains all hardware pin definitions, radio configuration,
 * and system constants for the remote phaser unit.
 *
 * Hardware: Adafruit Feather M0 with RFM95 LoRa Radio
 * Purpose: Remote antenna rotation control and telemetry
 *
 * @author Your Name
 * @date 2024
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// RADIO CONFIGURATION
// ============================================================================

/** @brief LoRa radio frequency in MHz */
#define RF95_FREQ 915.0

/** @brief This phaser unit's address */
#define MY_ADDRESS 212

/** @brief Controller unit's address */
#define CTRL_ADDRESS 211

/** @brief Radio chip select pin */
#define RF95_CS 8

/** @brief Radio interrupt pin */
#define RF95_INT 3

/** @brief Status LED pin */
#define LED 13

// ============================================================================
// RELAY CONTROL PINS
// ============================================================================

/** @brief Relay 1 control pin (Element 1 North) */
#define RELAY_1 6

/** @brief Relay 2 control pin (Element 2 South) */
#define RELAY_2 5

/** @brief Relay 3 control pin (Element 3 East) */
#define RELAY_3 10

/** @brief Relay 4 control pin (Element 4 West) */
#define RELAY_4 11

/** @brief Relay 5/6 parallel output pin */
#define RELAY_56 12

/** @brief Relay 7/8 parallel output pin */
#define RELAY_78 15

// ============================================================================
// SENSOR PINS
// ============================================================================

/** @brief Analog pin for reverse power measurement */
#define REV_POWER_PIN A2

/** @brief I2C address of INA3221 current/voltage monitor */
#define INA3221_I2C_ADDRESS 0x40

// ============================================================================
// ANTENNA CONFIGURATION SELECTION
// ============================================================================

/**
 * @brief Antenna controller type selection
 *
 * Choose one of:
 * - ANTENNA_REMOTEQTH (default) - 8-direction RemoteQTH controller
 * - ANTENNA_COMTEK - 4-direction Comtek controller
 *
 * Can be overridden at compile time via platformio.ini:
 * build_flags = -D ANTENNA_COMTEK
 */
#ifndef ANTENNA_CONFIG
    #define ANTENNA_CONFIG ANTENNA_REMOTEQTH
#endif

#define ANTENNA_REMOTEQTH 1
#define ANTENNA_COMTEK 2

// ============================================================================
// ANTENNA DIRECTIONS (8-Direction Enum)
// ============================================================================

/** @brief Antenna direction enumeration */
enum Direction {
    DIR_N = 0,    /**< North (000°) */
    DIR_NE = 1,   /**< Northeast (045°) */
    DIR_E = 2,    /**< East (090°) */
    DIR_SE = 3,   /**< Southeast (135°) */
    DIR_S = 4,    /**< South (180°) */
    DIR_SW = 5,   /**< Southwest (225°) */
    DIR_W = 6,    /**< West (270°) */
    DIR_NW = 7    /**< Northwest (315°) */
};

/** @brief Direction angles in degrees */
static const char* DIRECTION_ANGLES[] = {
    "000", "045", "090", "135", "180", "225", "270", "315"
};

/** @brief Number of directions */
#define NUM_DIRECTIONS 8

// ============================================================================
// RELAY CONFIGURATION - REMOTEQTH (8-Direction)
// ============================================================================

#if ANTENNA_CONFIG == ANTENNA_REMOTEQTH

/**
 * @brief Relay configuration for RemoteQTH 8-direction controller
 *
 * Each row represents relay states {R1, R2, R3, R4, R5/6, R7/8} for that direction
 */
static const boolean RELAY_POSITIONS[8][6] = {
    {LOW,  LOW,  LOW,  LOW,  LOW,  LOW},  // N (000°): 0
    {LOW,  LOW,  HIGH, HIGH, LOW,  HIGH}, // NE (045°): 1
    {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}, // E (090°): 2
    {LOW,  HIGH, HIGH, LOW,  LOW,  HIGH}, // SE (135°): 3
    {LOW,  LOW,  LOW,  LOW,  HIGH, HIGH}, // S (180°): 4
    {HIGH, HIGH, LOW,  LOW,  LOW,  HIGH}, // SW (225°): 5
    {HIGH, HIGH, HIGH, HIGH, LOW,  LOW},  // W (270°): 6
    {HIGH, LOW,  LOW,  HIGH, LOW,  LOW}   // NW (315°): 7
};

#elif ANTENNA_CONFIG == ANTENNA_COMTEK

/**
 * @brief Relay configuration for Comtek 4-direction controller
 *
 * Comtek only supports 4 directions using 2 relays:
 * - NE/N: Relays off/off
 * - SE/E: Relay 1 on
 * - SW/S: Relay 2 on  
 * - NW/W: Both relays on
 *
 * NE and E are combined, S and W are combined.
 */
static const boolean RELAY_POSITIONS[8][6] = {
    {LOW,  LOW,  LOW,  LOW,  LOW,  LOW},  // N (000°): Maps to NE pattern
    {LOW,  LOW,  LOW,  LOW,  LOW,  LOW},  // NE (045°): 0
    {HIGH, LOW,  LOW,  LOW,  LOW,  LOW},  // E (090°): Maps to SE pattern
    {HIGH, LOW,  LOW,  LOW,  LOW,  LOW},  // SE (135°): 1
    {LOW,  HIGH, LOW,  LOW,  LOW,  LOW},  // S (180°): Maps to SW pattern
    {LOW,  HIGH, LOW,  LOW,  LOW,  LOW},  // SW (225°): 2
    {HIGH, HIGH, LOW,  LOW,  LOW,  LOW},  // W (270°): Maps to NW pattern
    {HIGH, HIGH, LOW,  LOW,  LOW,  LOW}   // NW (315°): 3
};

#else
    #error "Invalid ANTENNA_CONFIG. Use ANTENNA_REMOTEQTH or ANTENNA_COMTEK"
#endif

// ============================================================================
// ADC CONFIGURATION FOR REVERSE POWER MEASUREMENT
// ============================================================================

/** @brief Number of ADC samples to average for SWR reading */
#define ADC_AVG_COUNT 10

/** @brief Delay between accumulating ADC samples (ms) */
#define ADC_SAMPLE_DELAY 10

/** @brief Conversion factor: ADC counts to volts for reverse power
 *
 * Depends on voltage divider in antenna unit.
 * RemoteQTH: 3.3V/1024 * count / divider_ratio = power_W
 */
#define REV_POWER_CONVERSION_FACTOR 0.5474F

// ============================================================================
// PROTOCOL CONFIGURATION
// ============================================================================

/** @brief Maximum command buffer length */
#define MAX_COMMAND_LEN 7

/** @brief Command buffer for receiving from controller */
extern char command_buffer[MAX_COMMAND_LEN];

/** @brief Actually received command length */
extern int command_length;

// ============================================================================
// MEASUREMENT AVERAGING
// ============================================================================

/** @brief Number of samples for current/voltage averaging */
#define INA_AVG_SAMPLES 16

// ============================================================================
// DEBUG CONFIGURATION
// ============================================================================

/** @brief Enable debug output to Serial */
#define DEBUG 0

#if DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
    #define DEBUG_PRINTLN(x) Serial.println(x)
    #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
    #define DEBUG_PRINT(x)
    #define DEBUG_PRINTLN(x)
    #define DEBUG_PRINTF(...)
#endif

#endif // CONFIG_H
