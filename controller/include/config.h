/**
 * @file config.h
 * @brief Configuration and hardware definitions for LoRa Antenna Controller
 *
 * This file contains all hardware pin definitions, radio configuration,
 * and system constants for the controller unit.
 *
 * Hardware: Adafruit Feather M0 with RFM95 LoRa Radio
 * Purpose: Remote antenna azimuth control from the shack
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
#define RF95_FREQ 915.0

/** @brief This controller's address */
#define MY_ADDRESS 211

/** @brief Remote phaser unit's address */
#define DEST_ADDRESS 212

/** @brief Radio chip select pin */
#define RF95_CS 8

/** @brief Radio interrupt pin */
#define RF95_INT 3

/** @brief Status LED pin */
#define LED 13

/** @brief Timeout waiting for reply from phaser (milliseconds) */
#define REC_TIMEOUT 1000

// ============================================================================
// GPIO EXPANDER (MCP23017) CONFIGURATION
// ============================================================================

/** @brief I2C address of MCP23017 GPIO expander (default address) */
#define MCP_I2C_ADDRESS 0x20

/** @brief Number of buttons/directions */
#define NUM_DIRECTIONS 8

/** @brief Button pins on MCP (pins 0-7) */
#define BUTTON_PIN_START 0
#define BUTTON_PIN_END 7

/** @brief LED pins on MCP (pins 8-15) */
#define LED_PIN_START 8
#define LED_PIN_END 15

// ============================================================================
// OLED DISPLAY CONFIGURATION
// ============================================================================

/** @brief I2C address of SH1106 OLED display */
#define OLED_I2C_ADDRESS 0x3C

/** @brief OLED display width in pixels */
#define SCREEN_WIDTH 128

/** @brief OLED display height in pixels */
#define SCREEN_HEIGHT 64

// ============================================================================
// INPUT/OUTPUT PINS
// ============================================================================

/** @brief PTT (Push-To-Talk) button pin for requesting antenna telemetry */
#define PTT_PIN 11

/** @brief Debounce delay for PTT input (milliseconds) */
#define DEBOUNCE_DELAY_MS 25

// ============================================================================
// ANTENNA DIRECTIONS
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

/** @brief Direction names for display */
static const char* DIRECTION_NAMES[] = {
    "N", "NE", "E", "SE", "S", "SW", "W", "NW"
};

/** @brief Direction angles in degrees */
static const int DIRECTION_ANGLES[] = {
    0, 45, 90, 135, 180, 225, 270, 315
};

// ============================================================================
// PROTOCOL CONFIGURATION
// ============================================================================

/** @brief Maximum length of command buffer */
#define MAX_COMMAND_LEN 7

/** @brief Maximum length of reply buffer */
#define MAX_REPLY_LEN 256

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
