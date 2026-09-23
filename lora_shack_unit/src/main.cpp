/**
 * @file main.cpp
 * @brief LoRa Antenna Controller (Heltec WiFi LoRa 32 V3)
 *
 * Controls antenna azimuth remotely via LoRa from the shack.
 *
 * Features:
 * - 8-direction antenna control (N, NE, E, SE, S, SW, W, NW)
 * - OLED display showing antenna status and telemetry
 * - Button interface for direction selection via MCP23017
 * - PTT input for requesting power/SWR telemetry
 * - Serial interface for manual command entry
 *
 * Hardware:
 * - Heltec WiFi LoRa 32 V3 (ESP32-S3)
 * - SX1262 LoRa Radio (915 MHz) via RadioLib
 * - SSD1306 OLED Display (built-in, Wire1)
 * - MCP23017 I2C GPIO Expander (Wire, SDA=41, SCL=42)
 *
 * @author Rajiv Dewan, N2RD
 * @date 2026
 */

// ============================================================================
// INCLUDES
// ============================================================================

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <RadioLib.h>
#include <Adafruit_MCP23X17.h>
#include <U8g2lib.h>
#include "config.h"
#include "protocol.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

// RadioLib SX1262: Module(cs, irq, rst, busy)
SX1262 radio = new Module(SX1262_CS, SX1262_INT, SX1262_RST, SX1262_BUSY);

// OLED: portrait mode (U8G2_R1 = 90° CW → 64×128), SW I2C on dedicated pins
U8G2_SSD1306_128X64_NONAME_F_SW_I2C display(U8G2_R2, I2C_SCL_OLED, I2C_SDA_OLED, OLED_RST);

// GPIO expander
Adafruit_MCP23X17 mcp;

// ============================================================================
// APPLICATION STATE
// ============================================================================

int current_direction  = DIR_NE;
int last_button_pressed = DIR_NE;
int16_t packet_count    = 0;
char cached_rev_pwr[8]  = "--";   // rev power from last PTT reply
char cached_voltage[12] = "--";   // voltage from last direction reply
char cached_current[8]  = "--";   // current from last direction reply
Command current_command = {{0}, 0};
uint8_t last_reply_buffer[255] = {0};
uint8_t last_reply_length = 0;

// ISR flag: set by DIO1 interrupt when a LoRa packet is received
volatile bool replyReceived = false;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

void init_all_hardware(void);
void build_direction_command(int direction, Command& cmd);
void build_ptt_command(Command& cmd);
void send_and_process_command(const Command& cmd);
void display_direction_reply(const uint8_t* buf, uint8_t len);
void display_power_reply(const uint8_t* buf, uint8_t len);
void handle_button_press(int button);
void handle_ptt_press(void);
void handle_serial_input(void);
void display_message(const char* message);
bool debounce_pin(int pin, int target_level);
int  angle_to_direction(int angle);
void blink_direction_led(int dir);

// ============================================================================
// INTERRUPT SERVICE ROUTINE
// ============================================================================

void IRAM_ATTR onPacketReceived(void) {
    replyReceived = true;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void init_all_hardware(void) {
    // Serial — USB CDC on ESP32-S3
    Serial.begin(115200);
    // Print dots for 5s so the terminal can connect before important messages
    for (int i = 0; i < 50; i++) {
        Serial.print('.');
        Serial.flush();
        delay(100);
    }
    Serial.println();
    Serial.println("\n========== LoRa Antenna Controller (Heltec V4) Starting ==========");
    Serial.printf("  CPU: %d MHz, Free heap: %d bytes\n",
                  (int)(F_CPU / 1000000), ESP.getFreeHeap());

    // Enable Vext (GPIO36 LOW) to power OLED and MCP23017
    Serial.println("[INIT] Enabling Vext peripheral power...");
    pinMode(VEXT_PIN, OUTPUT);
    digitalWrite(VEXT_PIN, LOW);
    delay(200);
    Serial.println("[INIT] Vext enabled.");

    // PTT pin
    pinMode(PTT_PIN, INPUT_PULLUP);
    Serial.printf("[INIT] PTT pin GPIO%d configured INPUT_PULLUP\n", PTT_PIN);

    // External I2C for MCP23017 (SDA=41, SCL=42 from variants.h)
    Serial.println("[INIT] Starting Wire for MCP23017...");
    Wire.begin();
    Serial.println("[INIT] Wire ready.");

    // RadioLib SX1262 init
    // begin(freq, bw, sf, cr, syncWord, pwr, preambleLen, tcxoVoltage)
    // TCXO on Heltec V3 is 1.8V — required for correct operation
    // set FM pins to output and set them high to enable PA on receive and transmit
    pinMode(SX1262_FEM_EN, OUTPUT);
    pinMode(SX1262_FEM_PA, OUTPUT);
    digitalWrite(SX1262_FEM_EN, HIGH);
    digitalWrite(SX1262_FEM_PA, HIGH);
    
    Serial.printf("[INIT] Starting LoRa radio (CS=%d, INT=%d, RST=%d, BUSY=%d)...\n",
                  SX1262_CS, SX1262_INT, SX1262_RST, SX1262_BUSY);
    //int state = radio.begin(SX1262_FREQ, 125.0, 9, 7,
    //                        RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 20, 8, 1.8);
    int state = radio.begin(SX1262_FREQ, 250.0, 11, 5,
                            RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 20, 8, 1.8);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("[INIT] ERROR: Radio init failed, code %d\n", state);
        while (1) {
            digitalWrite(LED, HIGH); delay(100);
            digitalWrite(LED, LOW);  delay(100);
        }
    }
    // DIO2 drives the RF switch on Heltec V3
    radio.setDio2AsRfSwitch(true);
    // Register ISR for received packets
    radio.setDio1Action(onPacketReceived);
    Serial.printf("[INIT] Radio OK: %.1f MHz, SF9, BW125, CR4/7, TX 20 dBm\n",
                  SX1262_FREQ);

    // OLED init
    pinMode(LED, OUTPUT);
    digitalWrite(LED, HIGH);
    Serial.println("[INIT] Starting OLED display...");
    display.begin();                          // handles reset internally
    display.setContrast(255);
    display.clearBuffer();
    display.setFont(u8g2_font_helvB12_tr);
    display.drawStr(2, 30, "READY!");
    display.sendBuffer();
    Serial.println("[INIT] OLED OK.");
    delay(500);

    // MCP23017 GPIO expander
    Serial.printf("[INIT] Starting MCP23017 (I2C addr 0x%02X)...\n", MCP_I2C_ADDRESS);
    if (!mcp.begin_I2C()) {
        Serial.println("ERROR: MCP23017 initialization failed!");
        display.clearBuffer();
        display.setFont(u8g2_font_helvB12_tr);
        display.drawStr(2, 30, "GPIO FAIL");
        display.sendBuffer();
        while (1);
    }
    // Pins 0-7: inputs (buttons), pins 8-15: outputs (LEDs)
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        mcp.pinMode(BUTTON_PIN_START + i, INPUT_PULLUP);
        mcp.pinMode(LED_PIN_START + i, OUTPUT);
    }
    // Explicitly clear all LED outputs (MCP output latch is undefined at power-on)
    for (int i = 0; i < NUM_DIRECTIONS; i++)
        mcp.digitalWrite(LED_PIN_START + i, LOW);
    current_direction   = DIR_NE;
    last_button_pressed = DIR_NE;
    mcp.digitalWrite(LED_PIN_START + DIR_TO_HW[DIR_NE], HIGH);
    Serial.println("[INIT] MCP23017 OK.");

    Serial.println("[INIT] ========== All systems ready ==========\n");
}

// ============================================================================
// COMMAND BUILDING
// ============================================================================

void build_direction_command(int direction, Command& cmd) {
    const char* angles[] = {
        ANGLE_NE, ANGLE_E, ANGLE_SE, ANGLE_S,
        ANGLE_SW, ANGLE_W, ANGLE_NW, ANGLE_N
    };
    cmd.data[0] = 'A';
    cmd.data[1] = 'P';
    cmd.data[2] = '1';
    cmd.data[3] = angles[direction][0];
    cmd.data[4] = angles[direction][1];
    cmd.data[5] = angles[direction][2];
    cmd.data[6] = CMD_TERMINATOR;
    cmd.length = 7;
    DEBUG_PRINTF("Built direction command for %s (%s deg)\n",
                 DIRECTION_NAMES[direction], angles[direction]);
}

void build_ptt_command(Command& cmd) {
    cmd.data[0] = CMD_PTT;
    cmd.length = 1;
    DEBUG_PRINTLN("Built PTT (telemetry) command");
}

// ============================================================================
// RADIO SEND / RECEIVE
// ============================================================================

// Called after a direction command reply (;XXXrRRRRvVVVVViIII).
// Updates the cached voltage and current; uses cached rev power.
void display_direction_reply(const uint8_t* buf, uint8_t len) {
    // Update cached voltage (buf[10..14]) and current (buf[16..18])
    if (len >= 15) {
        snprintf(cached_voltage, sizeof(cached_voltage), "%c%c.%c%c%c",
                 buf[10], buf[11], buf[12], buf[13], buf[14]);
    }
    if (len >= 19) {
        snprintf(cached_current, sizeof(cached_current), "%c%c%c",
                 buf[16], buf[17], buf[18]);
    }

    char line[24];
    display.clearBuffer();

    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(2, 10, "REV PWR");
    display.drawStr(66, 10, "POS");
    display.setFont(u8g2_font_helvB12_tr);
    display.drawStr(2, 28, cached_rev_pwr);        // reuse last PTT value
    display.drawStr(66, 28, DIRECTION_NAMES[current_direction]);

    display.drawHLine(0, 33, 128);

    display.setFont(u8g2_font_6x10_tf);
    snprintf(line, sizeof(line), "RSSI:%+d", (int)radio.getRSSI());
    display.drawStr(2, 46, line);
    snprintf(line, sizeof(line), "V:%s", cached_voltage);
    display.drawStr(2, 58, line);
    snprintf(line, sizeof(line), "mA:%s", cached_current);
    display.drawStr(66, 58, line);
    display.sendBuffer();
}

// Called after a PTT command reply (VPPPPP).
// Updates the cached rev power; uses cached voltage and current.
void display_power_reply(const uint8_t* buf, uint8_t len) {
    // Update cached rev power — skip the leading 'V' prefix byte
    if (len >= 2) {
        int copy_len = (int)len - 1;
        if (copy_len > (int)sizeof(cached_rev_pwr) - 1)
            copy_len = (int)sizeof(cached_rev_pwr) - 1;
        for (int i = 0; i < copy_len; i++)
            cached_rev_pwr[i] = (char)buf[i + 1];
        cached_rev_pwr[copy_len] = '\0';
    }

    char line[24];
    display.clearBuffer();

    display.setFont(u8g2_font_6x10_tf);
    display.drawStr(2, 10, "REV PWR");
    display.drawStr(66, 10, "POS");
    display.setFont(u8g2_font_helvB12_tr);
    display.drawStr(2, 28, cached_rev_pwr);
    display.drawStr(66, 28, DIRECTION_NAMES[current_direction]); // reuse last direction

    display.drawHLine(0, 33, 128);

    display.setFont(u8g2_font_6x10_tf);
    snprintf(line, sizeof(line), "RSSI:%+d", (int)radio.getRSSI());
    display.drawStr(2, 46, line);
    snprintf(line, sizeof(line), "V:%s", cached_voltage);        // reuse last direction value
    display.drawStr(2, 58, line);
    snprintf(line, sizeof(line), "mA:%s", cached_current);       // reuse last direction value
    display.drawStr(66, 58, line);

    display.sendBuffer();
}

void send_and_process_command(const Command& cmd) {
    Serial.printf("Sending command (%d bytes):", cmd.length);
    for (int i = 0; i < cmd.length; i++) Serial.printf(" %02X", cmd.data[i]);
    Serial.println();

    // Transmit (blocking — returns when TX complete or error)
    int state = radio.transmit(const_cast<uint8_t*>(cmd.data), cmd.length);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("ERROR: TX failed, code %d\n", state);
        display_message("SEND ERR");
        return;
    }

    // Arm receive and wait for phaser reply
    // Power queries need extra time: the phaser takes ~100 ms for ADC sampling
    bool is_power_cmd = (cmd.length == 1 && cmd.data[0] == CMD_PTT);
    replyReceived = false;
    radio.startReceive();

    unsigned long deadline = millis() + (is_power_cmd ? REC_TIMEOUT_POWER : REC_TIMEOUT);
    while (millis() < deadline) {
        if (replyReceived) break;
        delay(1);
    }

    if (!replyReceived) {
        radio.standby();
        Serial.println("No reply received (timeout)");
        display_message("TIMEOUT");
        return;
    }
    replyReceived = false;

    uint8_t reply_buf[255] = {0};
    size_t reply_len = radio.getPacketLength();
    if (reply_len > sizeof(reply_buf)) reply_len = sizeof(reply_buf);
    state = radio.readData(reply_buf, reply_len);

    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("ERROR: readData failed, code %d\n", state);
        display_message("READ ERR");
        return;
    }

    Serial.printf("Reply received (%d bytes), RSSI: %d dBm\n",
                  (int)reply_len, (int)radio.getRSSI());
    memcpy(last_reply_buffer, reply_buf, reply_len);
    last_reply_length = (uint8_t)reply_len;
    packet_count++;
    if (reply_len > 0 && reply_buf[0] == REPLY_POWER) {
        display_power_reply(reply_buf, (uint8_t)reply_len);
    } else {
        // Parse azimuth from reply bytes [1..3], update direction, blink LED
        if (reply_len >= 4 && reply_buf[0] == REPLY_POSITION) {
            int azimuth = (reply_buf[1] - '0') * 100 +
                          (reply_buf[2] - '0') * 10 +
                          (reply_buf[3] - '0');
            int dir = angle_to_direction(azimuth);
            if (dir >= 0) {
                current_direction = dir;
                blink_direction_led(dir);
            }
        }
        display_direction_reply(reply_buf, (uint8_t)reply_len);
    }
}

// ============================================================================
// USER INPUT HANDLING
// ============================================================================

void handle_button_press(int button) {
    if (button < 0 || button >= NUM_DIRECTIONS) return;
    if (button == last_button_pressed) return;

    Serial.printf("Button %d pressed: %s\n", button, DIRECTION_NAMES[button]);

    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        mcp.digitalWrite(LED_PIN_START + DIR_TO_HW[i], (i == button) ? HIGH : LOW);
    }

    current_direction = button;   // optimistic update; overridden by reply
    build_direction_command(button, current_command);
    send_and_process_command(current_command);
    last_button_pressed = button;
}

void handle_ptt_press(void) {
    Serial.println("PTT pressed: requesting reverse power telemetry");
    build_ptt_command(current_command);
    send_and_process_command(current_command);
}

void handle_serial_input(void) {
    static char serial_buffer[10];
    static int serial_index = 0;

    while (Serial.available() > 0) {
        int c = Serial.read();

        if (c == '\n' || c == '\r' || c == ' ') {
            if (serial_index > 0) {
                serial_buffer[serial_index] = '\0';
                int direction = -1;

                for (int i = 0; i < NUM_DIRECTIONS; i++) {
                    if (strcasecmp(serial_buffer, DIRECTION_NAMES[i]) == 0) {
                        direction = i;
                        break;
                    }
                }

                if (direction == -1) {
                    int angle = atoi(serial_buffer);
                    switch (angle) {
                        case 0: case 360: direction = DIR_N;  break;
                        case 45:          direction = DIR_NE; break;
                        case 90:          direction = DIR_E;  break;
                        case 135:         direction = DIR_SE; break;
                        case 180:         direction = DIR_S;  break;
                        case 225:         direction = DIR_SW; break;
                        case 270:         direction = DIR_W;  break;
                        case 315:         direction = DIR_NW; break;
                    }
                }

                if (direction >= 0) {
                    handle_button_press(direction);
                } else {
                    Serial.println("Unknown direction. Use: N NE E SE S SW W NW or angle 0-315");
                }
                serial_index = 0;
            }
            continue;
        }

        if (serial_index < (int)sizeof(serial_buffer) - 1) {
            serial_buffer[serial_index++] = (char)c;
        }
    }
}

// ============================================================================
// DISPLAY HELPERS
// ============================================================================

void display_message(const char* message) {
    display.clearBuffer();
    display.setFont(u8g2_font_helvB12_tr);
    display.drawStr(2, 50, message);
    display.sendBuffer();
    delay(1000);
}

// ============================================================================
// DIRECTION LED HELPERS
// ============================================================================

// Maps a standard azimuth (0/45/90/135/180/225/270/315) to a DIR_* index.
// Returns -1 if the angle is not a recognised direction.
int angle_to_direction(int angle) {
    switch (angle) {
        case 0: case 360: return DIR_N;
        case 45:          return DIR_NE;
        case 90:          return DIR_E;
        case 135:         return DIR_SE;
        case 180:         return DIR_S;
        case 225:         return DIR_SW;
        case 270:         return DIR_W;
        case 315:         return DIR_NW;
        default:          return -1;
    }
}

// Blink the LED for 'dir' twice (0.5 s cycle: 250 ms on / 250 ms off),
// then leave it lit.  All other LEDs are off throughout.
void blink_direction_led(int dir) {
    int hw = DIR_TO_HW[dir];  // translate direction index → hardware pin
    for (int i = 0; i < NUM_DIRECTIONS; i++)
        mcp.digitalWrite(LED_PIN_START + i, LOW);
    for (int b = 0; b < 2; b++) {
        mcp.digitalWrite(LED_PIN_START + hw, HIGH);
        delay(250);
        mcp.digitalWrite(LED_PIN_START + hw, LOW);
        delay(250);
    }
    mcp.digitalWrite(LED_PIN_START + hw, HIGH);
}

// ============================================================================
// MAIN SETUP AND LOOP
// ============================================================================

void setup() {
    init_all_hardware();
}

void loop() {
    // PTT button — highest priority
    if (debounce_pin(PTT_PIN, LOW)) {
        handle_ptt_press();
        while (debounce_pin(PTT_PIN, LOW)) { delay(10); }
        delay(100);
    }

    // Direction buttons on MCP23017
    for (int i = 0; i < NUM_DIRECTIONS; i++) {
        if (!mcp.digitalRead(BUTTON_PIN_START + i)) {
            handle_button_press(HW_TO_DIR[i]);  // translate hw pin → direction index
            delay(50);
            while (!mcp.digitalRead(BUTTON_PIN_START + i)) { delay(10); }
            delay(100);
        }
    }

    handle_serial_input();
    delay(10);
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

bool debounce_pin(int pin, int target_level) {
    int current_level = digitalRead(pin);
    for (int i = 0; i < DEBOUNCE_DELAY_MS; i++) {
        delay(1);
        if (digitalRead(pin) != current_level) return false;
    }
    return (digitalRead(pin) == target_level);
}
