/**
 * @file protocol.h
 * @brief LoRa antenna controller protocol definitions
 *
 * Implements DCU-1 compatible protocol for antenna azimuth control.
 * Commands are sent in ASCII format: AP1###<CR>
 * Replies contain position, RSSI, voltage, current, and telemetry data.
 *
 * Protocol Overview:
 * - Commands follow Yaesu DCU-1 antenna rotator protocol
 * - Responses include antenna position and power/current telemetry
 * - Position format: AP1###\r where ### is azimuth 000-359
 * - PTT command format: V (single character)
 *
 * @author Rajiv Dewan, N2RD
 * @date 2026
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

// ============================================================================
// COMMAND DEFINITIONS
// ============================================================================

/** @brief Prefix for position commands: "AP1" */
#define CMD_PREFIX_POS 'A'
#define CMD_PREFIX_POS2 'P'
#define CMD_PREFIX_POS3 '1'

/** @brief PTT (voltage/power report) command */
#define CMD_PTT 'V'

/** @brief Command terminator: carriage return */
#define CMD_TERMINATOR '\r'

/** @brief Alternative command terminator: semicolon (from some controllers) */
#define CMD_ALT_TERMINATOR ';'

// ============================================================================
// REPLY DEFINITIONS
// ============================================================================

/** @brief Position reply prefix indicating antenna position data */
#define REPLY_POSITION ';'

/** @brief Power/telemetry reply prefix indicating reverse power data */
#define REPLY_POWER 'V'

// ============================================================================
// DIRECTIONAL COMMAND CODES
// ============================================================================

/**
 * @brief Build a rotator angle from direction code
 *
 * Maps 8 directions to standard azimuth values
 * Used in protocol: AP1###\r where ### is 000-359
 */

// Full 8-direction angles (RemoteQTH format)
#define ANGLE_N "000"      /**< North: 000° */
#define ANGLE_NE "045"     /**< Northeast: 045° */
#define ANGLE_E "090"      /**< East: 090° */
#define ANGLE_SE "135"     /**< Southeast: 135° */
#define ANGLE_S "180"      /**< South: 180° */
#define ANGLE_SW "225"     /**< Southwest: 225° */
#define ANGLE_W "270"      /**< West: 270° */
#define ANGLE_NW "315"     /**< Northwest: 315° */

// ============================================================================
// COMMAND STRUCTURE
// ============================================================================

/** @brief Command buffer for transmitting to phaser */
struct Command {
    uint8_t data[7];       /**< Command bytes */
    uint8_t length;        /**< Valid command length */
};

// ============================================================================
// REPLY DATA STRUCTURE
// ============================================================================

/**
 * @brief Parsed reply from remote phaser unit
 *
 * Contains telemetry data received from the antenna controller:
 * - Position (azimuth angle)
 * - RSSI (signal strength for transmit and receive)
 * - Bus voltage and current
 * - MCU supply voltage
 */
struct PhaserReply {
    char position[4];      /**< Position string, e.g., "045" */
    int rssi_tx;           /**< Transmit RSSI (dBm) */
    int rssi_rx;           /**< Receive RSSI (dBm) */
    float bus_voltage;     /**< Bus voltage (V) */
    int bus_current;       /**< Bus current (mA) */
    float mcu_voltage;     /**< MCU supply voltage (V) */
    float rev_power;       /**< Reverse power (W) */
};

// ============================================================================
// SECURITY & AUTHENTICATION
// ============================================================================

/** 
 * @brief Shared authentication key for command validation
 * 
 * IMPORTANT: Change this to a unique value for your system.
 * Both controller and phaser must use identical keys.
 * Keep this secret to prevent unauthorized LoRa commands.
 */
#define AUTH_KEY "N2RD-ANTENNA-KEY"

/** @brief Length of authentication hash appended to commands (bytes) */
#define AUTH_LEN 2

/**
 * @brief Compute authentication hash for command
 * 
 * Simple hash-based authentication to prevent unauthorized commands.
 * Uses a basic mixing algorithm for minimal resource usage.
 *
 * @param data Command data buffer
 * @param len Length of command data (excluding auth bytes)
 * @return 16-bit authentication hash
 */
static inline uint16_t compute_auth(const uint8_t* data, uint8_t len) {
    uint16_t hash = 0xB33F;  // Start value
    for (uint8_t i = 0; i < len; i++) {
        // Simple mixing: rotate left by 5, XOR with key byte, add data
        hash = ((hash << 5) | (hash >> 11)) ^ ((uint8_t)AUTH_KEY[i % (sizeof(AUTH_KEY)-1)]);
        hash += data[i];
    }
    return hash;
}

#endif // PROTOCOL_H
