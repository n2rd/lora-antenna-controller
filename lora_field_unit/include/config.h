/**
 * @file config.h
 * @brief Configuration and hardware definitions for LoRa Antenna Phaser
 *
 * This file contains all hardware pin definitions, radio configuration,
 * and system constants for the remote phaser unit.
 *
 * Hardware: Heltec WiFi LoRa 32 V3 (ESP32-S3, SX1262)
 * Purpose: Remote antenna rotation control and telemetry
 *
 * @author Rajiv Dewan, N2RD
 * @date 2026
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// RADIO CONFIGURATION
// ============================================================================

/** @brief LoRa radio frequency in MHz */
#define SX1262_FREQ 915.0

/** @brief This phaser unit's address */
#define MY_ADDRESS 212

/** @brief Controller unit's address */
#define CTRL_ADDRESS 211

// Heltec WiFi LoRa 32 V4 — SX1262 radio pins
#define SX1262_CS   8   ///< LoRa NSS (SS)
#define SX1262_RST  12  ///< LoRa RST
#define SX1262_INT  14  ///< LoRa DIO1 (interrupt)
#define SX1262_BUSY 13  ///< LoRa BUSY
#define SX1262_FEM_EN 2 /// < RF FEM enable 
#define SX1262_FEM_PA 46 /// < RF TX/RX PA Enable

// Power and LED
#define LED      35  ///< Onboard LED (LED_BUILTIN)
#define VEXT_PIN 36  ///< Peripheral power control, active LOW

// External I2C bus (Wire) — used for INA219 (SDA=41, SCL=42 from variants.h)
#define I2C_SDA  41  ///< External I2C SDA
#define I2C_SCL  42  ///< External I2C SCL

// ============================================================================
// RELAY CONTROL PINS
// ============================================================================

/** @brief Relay 1 control pin */
#define RELAY_1  3

/** @brief Relay 2 control pin */
#define RELAY_2  4

/** @brief Relay 3 control pin */
#define RELAY_3  5

/** @brief Relay 4 control pin */
#define RELAY_4 33

/** @brief Relay 5/6 parallel output pin */
#define RELAY_56 47

/** @brief Relay 7/8 parallel output pin */
#define RELAY_78 48

// ============================================================================
// SENSOR PINS
// ============================================================================

/** @brief Analog pin for reverse power measurement (ADC1 Ch6, ESP32-S3) */
#define REV_POWER_PIN 6

/** @brief I2C address of INA219 current/voltage monitor */
#define INA219_I2C_ADDRESS 0x40

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
    DIR_N  = 0,   /**< North (000°) */
    DIR_NE = 1,   /**< Northeast (045°) */
    DIR_E  = 2,   /**< East (090°) */
    DIR_SE = 3,   /**< Southeast (135°) */
    DIR_S  = 4,   /**< South (180°) */
    DIR_SW = 5,   /**< Southwest (225°) */
    DIR_W  = 6,   /**< West (270°) */
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
    {HIGH, LOW,  LOW,  HIGH, LOW,  LOW},  // N  (000°): 0
    {LOW,  LOW,  LOW,  LOW,  LOW,  LOW},  // NE (045°): 1
    {LOW,  LOW,  HIGH, HIGH, LOW,  HIGH}, // E  (090°): 2
    {HIGH, HIGH, HIGH, HIGH, HIGH, HIGH}, // SE (135°): 3
    {LOW,  HIGH, HIGH, LOW,  LOW,  HIGH}, // S  (180°): 4
    {LOW,  LOW,  LOW,  LOW,  HIGH, HIGH}, // SW (225°): 5
    {HIGH, HIGH, LOW,  LOW,  LOW,  HIGH}, // W  (270°): 6
    {HIGH, HIGH, HIGH, HIGH, LOW,  LOW}   // NW (315°): 7
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

/** @brief Number of ADC samples taken per reverse power reading */
#define ADC_AVG_COUNT 10

/** @brief Delay between ADC samples (ms) — captures 24 ms RF pulses */
#define ADC_SAMPLE_DELAY 10

/** @brief Number of top samples to average for peak estimate */
#define ADC_TOP_COUNT 3

/**
 * @brief Voltage divider correction factor for reverse power measurement
 *
 * analogReadMilliVolts() returns the ADC pin voltage in mV (calibrated).
 * Multiply by this factor to recover the actual detector voltage in mV,
 * then divide by 1000 to obtain volts before computing power.
 */
#define REV_POWER_CONVERSION_FACTOR 371.71F

// ============================================================================
// PROTOCOL CONFIGURATION
// ============================================================================

/** @brief Maximum command buffer length (7 cmd bytes + 1 null) */
#define MAX_COMMAND_LEN 8

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

/** @brief Enable debug output to Serial (0=off, 1=on) */
#define DEBUG 1

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
