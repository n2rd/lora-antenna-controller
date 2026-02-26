/**
 * @file protocol.h
 * @brief LoRa antenna phaser protocol definitions
 *
 * Implements DCU-1 compatible protocol for antenna rotator control.
 * The phaser unit receives commands and returns telemetry including:
 * - Current antenna direction/azimuth
 * - Bus voltage and current
 * - RSSI (signal strength)
 * - MCU supply voltage
 * - Reverse power (SWR) measurement
 *
 * Protocol Commands Supported:
 * - AP1### or AP1###\r = Set antenna to azimuth ###
 * - AI1 or AI1; = Report current position
 * - V = Report reverse power/telemetry
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

/** @brief Position command prefix characters */
#define CMD_PREFIX_A 'A'
#define CMD_PREFIX_P 'P'
#define CMD_PREFIX_1 '1'

/** @brief Command terminators */
#define CMD_TERMINATOR_CR '\r'
#define CMD_TERMINATOR_SEMI ';'

// ============================================================================
// COMMAND TYPES
// ============================================================================

/** @brief PTT / Power request command */
#define CMD_TYPE_POWER 'V'

/** @brief Information request commands */
#define CMD_TYPE_INFO_M 'M'      // Execute movement
#define CMD_TYPE_INFO_I 'I'      // Report information/position
#define CMD_TYPE_INFO_S 'S'      // Set target (from AP1### format)

// ============================================================================
// REPLY DEFINITIONS
// ============================================================================

/** @brief Position reply prefix (semicolon) */
#define REPLY_PREFIX_POS ';'

/** @brief Power/voltage reply prefix (letter V) */
#define REPLY_PREFIX_PWR 'V'

/** @brief RSSI field marker in reply */
#define REPLY_FIELD_RSSI 'r'

/** @brief Voltage field marker */
#define REPLY_FIELD_VOLT 'v'

/** @brief Current field marker */
#define REPLY_FIELD_CURR 'i'

/** @brief Battery voltage field marker */
#define REPLY_FIELD_BATT 'b'

// ============================================================================
// REPLY STRUCTURE
// ============================================================================

/**
 * @brief Position reply format:
 *
 * ";XYZrRRRRvVVVVViIIIbBBBB"
 *
 * Where:
 * - ; = Position reply marker
 * - XYZ = 3-digit azimuth (000-359)
 * - r = RSSI field marker
 * - RRRR = RSSI value (e.g., -095)
 * - v = Voltage field marker
 * - VVVV = 5-digit bus voltage (e.g., 13800 = 13.8V)
 * - i = Current field marker
 * - III = 3-digit current in mA (e.g., 500)
 * - b = Battery field marker
 * - BBBB = 4-digit battery voltage (e.g., 4200 = 4.2V)
 */

/**
 * @brief Power report reply format:
 *
 * "VPPPPPP"
 *
 * Where:
 * - V = Power reply marker
 * - PPPPPP = 6 characters of power/voltage reading (e.g., "1500.6" = 1500.6W)
 */

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
