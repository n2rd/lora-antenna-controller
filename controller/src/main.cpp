/**
 * @file main.cpp
 * @brief LoRa Antenna Controller - Main Application
 *
 * Controls antenna azimuth remotely via LoRa from the shack.
 *
 * Features:
 * - 8-direction antenna control (N, NE, E, SE, S, SW, W, NW)
 * - OLED display showing antenna status and telemetry
 * - Button interface for direction selection via MCP23017
 * - PTT input for requesting power/SWR telemetry
 * - Serial interface for manual command entry
 * - RSSI display for link quality assessment
 *
 * Hardware:
 * - Adafruit Feather M0
 * - RFM95W LoRa Radio (915 MHz)
 * - SH1106 1.3" OLED Display
 * - MCP23017 I2C GPIO Expander
 *
 * @author Your Name
 * @date 2024
 */

// ============================================================================
// INCLUDES
// ============================================================================

// Standard Arduino libraries
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

// Hardware-specific libraries
#include <RH_RF95.h>
#include <RHReliableDatagram.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// Project headers
#include "config.h"
#include "protocol.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

// LoRa radio objects
RH_RF95 rf95(RF95_CS, RF95_INT);
RHReliableDatagram rf95_manager(rf95, MY_ADDRESS);

// Display objects
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// GPIO expander object
Adafruit_MCP23X17 mcp;

// ============================================================================
// APPLICATION STATE
// ============================================================================

/** @brief Current antenna direction (0-7) */
int current_direction = DIR_N;

/** @brief Last direction button pressed */
int last_button_pressed = DIR_N;

/** @brief Counter for packets sent (for monitoring) */
int16_t packet_count = 0;

/** @brief Last reverse power reading from phaser */
char last_rev_power[8] = "--";

/** @brief Command buffer for current transmission */
Command current_command = {{0}, 0};

/** @brief Buffer for last phaser reply */
uint8_t last_reply_buffer[RH_RF95_MAX_MESSAGE_LEN];
uint8_t last_reply_length = 0;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

void init_all_hardware(void);
void build_direction_command(int direction, Command& cmd);
void build_ptt_command(Command& cmd);
void send_and_process_command(const Command& cmd);
void process_reply(const uint8_t* buf, uint8_t len);
Direction parse_direction_from_reply(const uint8_t* buf);
void display_telemetry(const uint8_t* buf, uint8_t len);
void handle_button_press(int button);
void handle_ptt_press(void);
void handle_serial_input(void);

// ============================================================================
// INITIALIZATION
// ============================================================================

/**
 * @brief Initialize all hardware subsystems
 *
 * Sets up:
 * - Serial communications
 * - LoRa radio
 * - OLED display
 * - GPIO expander with buttons and LEDs
 *
 * Halts on critical hardware failures.
 */
void init_all_hardware(void) {
    // Initialize serial for debugging
    Serial.begin(4800);  // DCU1 controller uses 4800 baud
    delay(1000);
    Serial.println("\n========== LoRa Antenna Controller Starting ==========");
    
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
    
    // Configure radio frequency and power
    if (!rf95.setFrequency(RF95_FREQ)) {
        Serial.println("ERROR: Failed to set radio frequency!");
        while (1);
    }
    rf95.setTxPower(20, false);
    rf95_manager.setTimeout(REC_TIMEOUT);
    Serial.printf("✓ Radio configured: %.1f MHz, TX Power 20 dBm\n", RF95_FREQ);
    
    // Initialize OLED display
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    delay(250);  // Wait for OLED to power up
    
    if (!display.begin(OLED_I2C_ADDRESS, true)) {
        Serial.println("ERROR: OLED display initialization failed!");
        while (1);
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("READY!");
    display.display();
    Serial.println("✓ OLED Display initialized");
    delay(1000);
    
    // Initialize GPIO expander (buttons and LEDs)
    if (!mcp.begin_I2C()) {
        Serial.println("ERROR: MCP23017 GPIO expander initialization failed!");
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println("GPIO FAILED");
        display.display();
        while (1);
    }
    
    // Configure MCP pins: 0-7 as inputs (buttons), 8-15 as outputs (LEDs)
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        mcp.pinMode(BUTTON_PIN_START + i, INPUT_PULLUP);
        mcp.pinMode(LED_PIN_START + i, OUTPUT);
    }
    
    // Light up the LED for current direction
    mcp.digitalWrite(LED_PIN_START + current_direction, HIGH);
    Serial.println("✓ GPIO Expander initialized");
    
    Serial.println("========== All systems ready ==========\n");
}

// ============================================================================
// COMMAND BUILDING
// ============================================================================

/**
 * @brief Build a direction command for the phaser unit
 *
 * Builds command in format: AP1###\r
 * where ### is the azimuth angle (000-359)
 *
 * @param direction Direction code (0-7)
 * @param cmd Output command structure to fill
 */
void build_direction_command(int direction, Command& cmd) {
    const char* angles[] = {
        ANGLE_N, ANGLE_NE, ANGLE_E, ANGLE_SE,
        ANGLE_S, ANGLE_SW, ANGLE_W, ANGLE_NW
    };
    
    cmd.data[0] = 'A';
    cmd.data[1] = 'P';
    cmd.data[2] = '1';
    cmd.data[3] = angles[direction][0];
    cmd.data[4] = angles[direction][1];
    cmd.data[5] = angles[direction][2];
    cmd.data[6] = CMD_TERMINATOR;
    cmd.length = 7;
    
    DEBUG_PRINTF("Built direction command for %s (%s°)\n",
                 DIRECTION_NAMES[direction], angles[direction]);
}

/**
 * @brief Build a PTT (Power Telemetry) command
 *
 * Builds command for requesting reverse power and telemetry: V
 *
 * @param cmd Output command structure to fill
 */
void build_ptt_command(Command& cmd) {
    cmd.data[0] = CMD_PTT;
    cmd.length = 1;
    DEBUG_PRINTLN("Built PTT (telemetry) command");
}
  // write all details to OLED screen
  //first refresh the screen and REV PWR display
  display.clearDisplay();  //clear the screen first
  display.display();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.print("Rev ");
  for (int i=0;i<6;i++) display.print(rev_pwr[i]);
  display.println();
  // now write the transmit and receive rssi
  display.setCursor(0, 20);
  display.setTextSize(1);
  display.print("RSSI T/R: ");
  display.printf("%+04d",rf95.lastRssi());
  display.print("/");
  display.print("-");
  for (int i=6;i<9;i++){  //the position
    display.print((char)buf[i]);
  }
  //now for the voltage and current
  display.setCursor(0, 30);
  display.print("Bus V: ");
  display.print((char)buf[10]);
  display.print((char)buf[11]);
  display.print(".");
  for (int i=12;i<15;i++){  //the position
    display.print((char)buf[i]);
  }
  display.setCursor(0, 40);
  display.print("Bus mA: ");
  for (int i=16;i<19;i++){  //the position
    display.print((char)buf[i]);
  }
  display.setCursor(0, 50);
  display.print("MCU V: ");
  display.print((char)buf[20]);
  display.print(".");
  for (int i=21;i<24;i++){  //the position
    display.print((char)buf[i]);
  }
  //also print the position
    display.println("");
  }
  
  // Show RSSI
  display.setTextSize(1);
  display.print("RSSI: ");
  display.printf("%+d", rf95.lastRssi());
  display.println(" dBm");
  
  // Show current direction
  if (buf[0] == REPLY_POSITION && len >= 4) {
    display.print("Pos: ");
    display.print((char)buf[1]);
    display.print((char)buf[2]);
    display.println((char)buf[3]);
  }
  
  // Show antenna name
  display.printf("Dir: %s", DIRECTION_NAMES[current_direction]);
  
  display.display();
}

// ============================================================================
// USER INPUT HANDLING
// ============================================================================

/**
 * @brief Handle button press on GPIO expander
 *
 * @param button Button index (0-7)
 */
void handle_button_press(int button) {
  if (button < 0 || button >= NUM_DIRECTIONS) return;
  if (button == last_button_pressed) return;  // Ignore same button
  
  Serial.printf("Button %d pressed: %s\n", button, DIRECTION_NAMES[button]);
  
  // Turn off prev LED, turn on new LED
  for (int i = 0; i < NUM_DIRECTIONS; i++) {
    mcp.digitalWrite(LED_PIN_START + i, (i == button) ? HIGH : LOW);
  }
  
  // Send direction command
  build_direction_command(button, current_command);
  send_and_process_command(current_command);
  
  last_button_pressed = button;
}

/**
 * @brief Handle PTT button press
 *
 * PTT sends a telemetry request command ('V')
 */
void handle_ptt_press(void) {
  Serial.println("PTT pressed: requesting reverse power telemetry");
  
  build_ptt_command(current_command);
  send_and_process_command(current_command);
}

/**
 * @brief Handle serial input for remote control
 *
 * Allows entering commands from serial terminal:
 * - Enter direction strings: N, NE, E, SE, S, SW, W, NW
 * - Or angles: 000, 045, 090, 135, 180, 225, 270, 315
 */
void handle_serial_input(void) {
  static char serial_buffer[10];
  static int serial_index = 0;
  
  while (Serial.available() > 0) {
    int c = Serial.read();
    
    // Skip whitespace and newlines
    if (c == '\n' || c == '\r' || c == ' ') {
      if (serial_index > 0) {
        serial_buffer[serial_index] = '\0';
        
        // Parse direction name or angle
        int direction = -1;
        
        // Try to match direction name
        for (int i = 0; i < NUM_DIRECTIONS; i++) {
          if (strcasecmp(serial_buffer, DIRECTION_NAMES[i]) == 0) {
            direction = i;
            break;
          }
        }
        
        // If not matched, try to parse angle
        if (direction == -1) {
          int angle = atoi(serial_buffer);
          switch (angle) {
            case 0: case 360: direction = DIR_N; break;
            case 45: direction = DIR_NE; break;
            case 90: direction = DIR_E; break;
            case 135: direction = DIR_SE; break;
            case 180: direction = DIR_S; break;
            case 225: direction = DIR_SW; break;
            case 270: direction = DIR_W; break;
            case 315: direction = DIR_NW; break;
          }
        }
        
        if (direction >= 0) {
          handle_button_press(direction);
        } else {
          Serial.println("Unknown direction. Use: N NE E SE S SW W NW or angles 0-359");
        }
        
        serial_index = 0;
      }
      continue;
    }
    
    // Add character to buffer
    if (serial_index < (int)sizeof(serial_buffer) - 1) {
      serial_buffer[serial_index++] = (char)c;
    }
  }
}

// ============================================================================
// DISPLAY HELPERS
// ============================================================================

/**
 * @brief Display a short status message
 *
 * @param message Message to display (clears after display)
 */
void display_message(const char* message) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 25);
  display.println(message);
  display.display();
  delay(1000);
}


// ============================================================================
// MAIN SETUP AND LOOP
// ============================================================================

void setup() {
  init_all_hardware();
}

void loop() {
  // Check PTT button (highest priority)
  if (debounce_pin(PTT_PIN, LOW)) {
    handle_ptt_press();
    
    // Stay in loop while PTT held to prevent hotswitch
    while (debounce_pin(PTT_PIN, LOW)) {
      delay(10);
    }
    delay(100);  // Debounce release
  }
  
  // Check direction buttons on GPIO expander
  for (int i = 0; i < NUM_DIRECTIONS; i++) {
    if (!mcp.digitalRead(BUTTON_PIN_START + i)) {  // Button pressed (active LOW)
      handle_button_press(i);
      delay(50);  // Debounce
      
      // Wait for release
      while (!mcp.digitalRead(BUTTON_PIN_START + i)) {
        delay(10);
      }
      delay(100);  // Debounce release
    }
  }
  
  // Check for serial input
  handle_serial_input();
  
  // Small delay to prevent CPU spinning
  delay(10);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * @brief Debounce a digital input pin
 *
 * Waits for pin to be stable at target level for DEBOUNCE_DELAY_MS.
 * Returns immediately if pin changes during waiting.
 *
 * @param pin Pin to read
 * @param target_level Target level (HIGH or LOW)
 * @return true if pin stabilized at target level
 */
bool debounce_pin(int pin, int target_level) {
  int current_level = digitalRead(pin);
  
  for (int i = 0; i < DEBOUNCE_DELAY_MS; i++) {
    delay(1);
    int new_level = digitalRead(pin);
    
    if (new_level != current_level) {
      return false;  // Level changed, not stable
    }
  }
  
  // Pin stabilized, return whether it matches target
  return (digitalRead(pin) == target_level);
}

