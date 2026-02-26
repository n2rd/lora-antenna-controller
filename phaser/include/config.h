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
 * @author Rajiv Dewan, N2RD
 * @date 2026
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>  // For uint8_t type

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
 * 0 = LOW (relay off), 1 = HIGH (relay on)
 */
static const uint8_t RELAY_POSITIONS[8][6] = {
    {0, 0, 0, 0, 0, 0},  // N (000°): 0
    {0, 0, 1, 1, 0, 1},  // NE (045°): 1
    {1, 1, 1, 1, 1, 1},  // E (090°): 2
    {0, 1, 1, 0, 0, 1},  // SE (135°): 3
    {0, 0, 0, 0, 1, 1},  // S (180°): 4
    {1, 1, 0, 0, 0, 1},  // SW (225°): 5
    {1, 1, 1, 1, 0, 0},  // W (270°): 6
    {1, 0, 0, 1, 0, 0}   // NW (315°): 7
};

#elif ANTENNA_CONFIG == ANTENNA_COMTEK

/**
 * @brief Relay configuration for Comtek 4-direction controller
 *
 * Comtek only supports 4 directions using 2 relays:
 * - N/NE: Relays off/off
 * - E/SE: Relay 1 on
 * - S/SW: Relay 2 on  
 * - W/NW: Both relays on
 *
 * The 8-element array accommodates the full direction range,
 * but indices are mapped: 0/1=N, 2/3=E, 4/5=S, 6/7=W
 */
static const uint8_t RELAY_POSITIONS[8][6] = {
    {0, 0, 0, 0, 0, 0},  // N (000°): Maps to NE pattern
    {0, 0, 0, 0, 0, 0},  // NE (045°): 0
    {1, 0, 0, 0, 0, 0},  // E (090°): Maps to SE pattern
    {1, 0, 0, 0, 0, 0},  // SE (135°): 1
    {0, 1, 0, 0, 0, 0},  // S (180°): Maps to SW pattern
    {0, 1, 0, 0, 0, 0},  // SW (225°): 2
    {1, 1, 0, 0, 0, 0},  // W (270°): Maps to NW pattern
    {1, 1, 0, 0, 0, 0}   // NW (315°): 3
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
