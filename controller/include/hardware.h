/**
 * @file hardware.h
 * @brief Hardware interface declarations for LoRa antenna controller
 *
 * Declares functions for managing:
 * - LoRa radio communication
 * - OLED display output
 * - MCP23017 GPIO expander (buttons and LEDs)
 * - User input/output operations
 *
 * @author Rajiv Dewan, N2RD
 * @date 2026
 */

#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>
#include "config.h"
#include "protocol.h"

// ============================================================================
// RADIO FUNCTIONS
// ============================================================================

/**
 * @brief Initialize the LoRa radio module
 *
 * Sets up the RH_RF95 radio driver and configures:
 * - Frequency (915.0 MHz)
 * - Transmission power (20 dBm)
 * - Reliable datagram manager for automatic ACKs
 *
 * @return true if initialization successful, false otherwise
 */
bool radio_init(void);

/**
 * @brief Send a command to the phaser and wait for reply
 *
 * Transmits a command via LoRa and waits for acknowledgment and reply.
 * Timeout is REC_TIMEOUT milliseconds.
 *
 * @param cmd Command to send
 * @return true if reply received, false if timeout or transmission error
 */
bool radio_send_and_receive(const Command& cmd);

/**
 * @brief Get the last received phaser reply data
 *
 * Returns pointer to buffer containing the most recent reply from phaser.
 * Buffer is valid until next radio_send_and_receive() call.
 *
 * @param len Output parameter: length of reply data
 * @return Pointer to reply buffer (uint8_t array)
 */
uint8_t* radio_get_last_reply(uint8_t& len);

/**
 * @brief Get the RSSI of the last transmission
 *
 * @return RSSI in dBm (negative value)
 */
int radio_get_last_rssi(void);

// ============================================================================
// DISPLAY FUNCTIONS (OLED)
// ============================================================================

/**
 * @brief Initialize the OLED display
 *
 * Sets up the SH1106 1.3" OLED display via I2C.
 * Displays "READY!" message after initialization.
 *
 * @return true if initialization successful, false otherwise
 */
bool display_init(void);

/**
 * @brief Display reverse power reading from antenna
 *
 * Shows on OLED:
 * - Reverse power (W)
 * - Transmit/Receive RSSI (dB)
 * - Bus voltage and current
 * - MCU supply voltage
 * - Current antenna position
 *
 * @param reply Reply data from phaser
 * @param rev_power Reverse power reading (string)
 */
void display_show_telemetry(uint8_t* reply_buf, uint8_t reply_len);

/**
 * @brief Display a status message on the OLED
 *
 * @param message Message to display (max ~20 chars)
 */
void display_message(const char* message);

/**
 * @brief Clear the OLED display
 */
void display_clear(void);

// ============================================================================
// GPIO EXPANDER FUNCTIONS (MCP23017)
// ============================================================================

/**
 * @brief Initialize the MCP23017 I2C GPIO expander
 *
 * Configures:
 * - Pins 0-7 as inputs with pull-ups (buttons)
 * - Pins 8-15 as outputs (LEDs)
 *
 * @return true if initialization successful, false otherwise
 */
bool gpio_init(void);

/**
 * @brief Get button press state
 *
 * @param button Button index (0-7, corresponding to N, NE, E, SE, S, SW, W, NW)
 * @return true if button is pressed (LOW), false otherwise
 */
bool gpio_read_button(int button);

/**
 * @brief Set LED state
 *
 * @param led LED index (0-7, corresponding to direction 0-7)
 * @param state true for ON (HIGH), false for OFF (LOW)
 */
void gpio_set_led(int led, bool state);

/**
 * @brief Turn off all LEDs
 */
void gpio_all_leds_off(void);

/**
 * @brief Blink an LED (blocking operation)
 *
 * @param led LED index (0-7)
 * @param delay_ms Delay between on/off (milliseconds)
 * @param count Number of blink cycles
 */
void gpio_blink_led(int led, int delay_ms, int count);

// ============================================================================
// INPUT/OUTPUT UTILITIES
// ============================================================================

/**
 * @brief Debounce a digital input pin
 *
 * Waits for the pin to stabilize at the specified level
 * for DEBOUNCE_DELAY_MS milliseconds.
 *
 * @param pin Pin number to debounce
 * @param target_level Target level (HIGH=1 or LOW=0)
 * @return true if pin stabilized at target level
 */
bool debounce_pin(int pin, int target_level);

/**
 * @brief Check if PTT button is pressed and debounced
 *
 * @return true if PTT pin is LOW (pressed) for DEBOUNCE_DELAY_MS
 */
bool ptt_pressed(void);

#endif // HARDWARE_H
