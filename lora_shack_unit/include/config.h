/**
 * @file config.h
 * @brief Configuration and hardware definitions for LoRa Antenna Controller
 *
 * This file contains all hardware pin definitions, radio configuration,
 * and system constants for the controller unit.
 *
 * Hardware: Heltec WiFi LoRa 32 V3 (ESP32-S3, SX1262)
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
#define SX1262_FREQ 915.0

/** @brief This controller's address */
#define MY_ADDRESS 211

/** @brief Remote phaser unit's address */
#define DEST_ADDRESS 212

// Heltec WiFi LoRa 32 V3 — SX1262 radio pins
#define SX1262_CS   8   ///< LoRa NSS (SS)
#define SX1262_RST  12  ///< LoRa RST
#define SX1262_INT  14  ///< LoRa DIO1 (interrupt)
#define SX1262_BUSY 13  ///< LoRa BUSY
#define SX1262_FEM_EN 2 /// < RF FEM enable 
#define SX1262_FEM_PA 46 /// < RF TX/RX PA Enable
// Power and LED
#define LED          35  ///< Onboard LED (LED_BUILTIN)
#define VEXT_PIN     36  ///< Peripheral power control, active LOW
// PTT input
#define PTT_PIN      3   ///< GPIO3, INPUT_PULLUP, active LOW
// External I2C bus (Wire) — used for MCP23017 (SDA=41, SCL=42 from variants.h)
#define I2C_SDA      41  ///< External I2C SDA
#define I2C_SCL      42  ///< External I2C SCL
// Onboard OLED I2C bus (Wire1) — SSD1306 on dedicated pins
#define I2C_SDA_OLED 17  ///< OLED SDA
#define I2C_SCL_OLED 18  ///< OLED SCL
#define OLED_RST     21  ///< OLED RST

// Transmitting 32B at SF 11 can take upto 2.2 seconds.  Use a longer timeout to be safe.
/** @brief Timeout waiting for reply from phaser (milliseconds) */
#define REC_TIMEOUT 5000

/** @brief Longer timeout for power/telemetry query — phaser needs ~100 ms for ADC sampling */
#define REC_TIMEOUT_POWER 5000

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

/** @brief Debounce delay for PTT input (milliseconds) */
#define DEBOUNCE_DELAY_MS 25

// ============================================================================
// ANTENNA DIRECTIONS
// ============================================================================

/** @brief Antenna direction enumeration */
enum Direction {
    DIR_NE = 0,   /**< Northeast (045°) */
    DIR_E = 1,    /**< East (090°) */
    DIR_SE = 2,   /**< Southeast (135°) */
    DIR_S = 3,    /**< South (180°) */
    DIR_SW = 4,   /**< Southwest (225°) */
    DIR_W = 5,    /**< West (270°) */
    DIR_NW = 6,   /**< Northwest (315°) */
    DIR_N = 7     /**< North (000°) */
};

/** @brief Direction names for display */
static const char* DIRECTION_NAMES[] = {
    "NE", "E", "SE", "S", "SW", "W", "NW", "N"
};

/** @brief Direction angles in degrees */
static const int DIRECTION_ANGLES[] = {
    45, 90, 135, 180, 225, 270, 315, 0
};

/**
 * @brief Physical MCP23017 pin layout starts at N=0 going clockwise:
 *   pin 0=N, 1=NE, 2=E, 3=SE, 4=S, 5=SW, 6=W, 7=NW
 * These tables translate between software direction index and hardware pin index.
 */
/** @brief Software direction index → hardware pin offset (0-7) */
static const int DIR_TO_HW[8] = {1, 2, 3, 4, 5, 6, 7, 0};
/** @brief Hardware pin offset (0-7) → software direction index */
static const int HW_TO_DIR[8] = {7, 0, 1, 2, 3, 4, 5, 6};

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
