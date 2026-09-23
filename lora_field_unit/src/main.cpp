/**
 * @file main.cpp
 * @brief LoRa Antenna Phaser - Remote Antenna Rotator Control (Heltec WiFi LoRa 32 V3)
 *
 * Receives LoRa commands from the controller, drives relay outputs to
 * switch antenna direction, and returns telemetry (position, voltage,
 * current, RSSI, reverse power).
 *
 * Hardware:
 * - Heltec WiFi LoRa 32 V3 (ESP32-S3)
 * - SX1262 LoRa Radio (915 MHz) via RadioLib
 * - 6-channel relay module on GPIO 1-6
 * - Adafruit INA219 on Wire (SDA=41, SCL=42)
 * - ADC input GPIO7 for reverse power (SWR)
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
#include <Adafruit_INA219.h>
#include "config.h"
#include "protocol.h"

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

// RadioLib SX1262: Module(cs, irq, rst, busy)
SX1262 radio = new Module(SX1262_CS, SX1262_INT, SX1262_RST, SX1262_BUSY);

// Current/voltage monitor
Adafruit_INA219 ina219;

// ============================================================================
// APPLICATION STATE
// ============================================================================

int current_direction = DIR_NE;
int target_direction  = DIR_NE;
int bus_voltage_mv    = 0;
int bus_current_ma    = 0;

char    command_buffer[MAX_COMMAND_LEN];
int     command_length = 0;
uint8_t reply_buffer[255];
int     reply_length  = 0;

int16_t packet_count  = 0;

bool have_ina219 = false;   ///< true if INA219 was detected at startup

// ISR flag: set by DIO1 when a LoRa packet arrives
volatile bool packetReceived = false;

// ============================================================================
// FUNCTION PROTOTYPES
// ============================================================================

void init_all_hardware(void);
void set_antenna_direction(int direction);
void measure_sensors(void);
int  read_reverse_power_adc(void);
int  read_bus_voltage(void);
int  read_bus_current(void);
void build_position_reply(int direction);
void build_power_reply(void);
void process_command(void);
void handle_set_direction(int direction);
void handle_position_query(void);
void handle_power_query(void);
int  parse_direction_from_command(void);

// ============================================================================
// INTERRUPT SERVICE ROUTINE
// ============================================================================

void IRAM_ATTR onPacketReceived(void) {
    packetReceived = true;
}

// ============================================================================
// INITIALIZATION
// ============================================================================

void init_all_hardware(void) {
    // Serial — USB CDC on ESP32-S3.
    // Do NOT block on Serial or flush when running without a USB host.
    Serial.begin(115200);
    delay(500);  // Short settle; skip the 5s dot loop when no host is connected
    Serial.println();
    Serial.println("\n========== LoRa Antenna Phaser (Heltec V3) Starting ==========");
    Serial.printf("  CPU: %d MHz, Free heap: %d bytes\n",
                  (int)(F_CPU / 1000000), ESP.getFreeHeap());

    // Enable Vext for external peripherals
    Serial.println("[INIT] Enabling Vext peripheral power...");
    pinMode(VEXT_PIN, OUTPUT);
    digitalWrite(VEXT_PIN, LOW);
    delay(200);
    Serial.println("[INIT] Vext enabled.");

    // LED
    pinMode(LED, OUTPUT);
    digitalWrite(LED, LOW);

    // Relay output pins — default all LOW (relays off)
    pinMode(RELAY_1,  OUTPUT); digitalWrite(RELAY_1,  LOW);
    pinMode(RELAY_2,  OUTPUT); digitalWrite(RELAY_2,  LOW);
    pinMode(RELAY_3,  OUTPUT); digitalWrite(RELAY_3,  LOW);
    pinMode(RELAY_4,  OUTPUT); digitalWrite(RELAY_4,  LOW);
    pinMode(RELAY_56, OUTPUT); digitalWrite(RELAY_56, LOW);
    pinMode(RELAY_78, OUTPUT); digitalWrite(RELAY_78, LOW);

    // Set initial safe direction
    set_antenna_direction(DIR_NE);
    Serial.println("[INIT] Relay outputs configured.");

    // ADC — 12-bit on ESP32-S3
    analogReadResolution(12);
    Serial.println("[INIT] ADC 12-bit resolution set.");

    // External I2C for INA219
    Wire.begin(I2C_SDA, I2C_SCL);

    // RadioLib SX1262 init
    // begin(freq, bw, sf, cr, syncWord, pwr, preambleLen, tcxoVoltage)
    // TCXO on Heltec V3 is 1.8V
    // set FM pins to output and set them high to enable PA on receive and transmit
    pinMode(SX1262_FEM_EN, OUTPUT);
    pinMode(SX1262_FEM_PA, OUTPUT);
    digitalWrite(SX1262_FEM_EN, HIGH);
    digitalWrite(SX1262_FEM_PA, HIGH);

    Serial.printf("[INIT] Starting LoRa radio (CS=%d, INT=%d, RST=%d, BUSY=%d)...\n",
                  SX1262_CS, SX1262_INT, SX1262_RST, SX1262_BUSY);
    int state = radio.begin(SX1262_FREQ, 250.0, 11, 5,
                            RADIOLIB_SX126X_SYNC_WORD_PRIVATE, 22, 8, 1.8);
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
    // Start listening
    radio.startReceive();
    Serial.printf("[INIT] Radio OK: %.1f MHz, SF9, BW125, CR4/7, TX 20 dBm\n",
                  SX1262_FREQ);

    // INA219 current/voltage monitor
    have_ina219 = ina219.begin();
    if (have_ina219) {
        Serial.println("[INIT] INA219 OK.");
    } else {
        Serial.println("[INIT] INA219 not found; telemetry returns zeros.");
    }

    // Initial sensor reading
    measure_sensors();

    Serial.println("[INIT] ========== All systems ready ==========\n");
}

// ============================================================================
// RELAY CONTROL
// ============================================================================

void set_antenna_direction(int direction) {
    if (direction < 0 || direction >= NUM_DIRECTIONS) return;

    digitalWrite(RELAY_1,  RELAY_POSITIONS[direction][0]);
    digitalWrite(RELAY_2,  RELAY_POSITIONS[direction][1]);
    digitalWrite(RELAY_3,  RELAY_POSITIONS[direction][2]);
    digitalWrite(RELAY_4,  RELAY_POSITIONS[direction][3]);
    digitalWrite(RELAY_56, RELAY_POSITIONS[direction][4]);
    digitalWrite(RELAY_78, RELAY_POSITIONS[direction][5]);

    current_direction = direction;
    DEBUG_PRINTF("Antenna direction set to %d (%s deg)\n",
                 direction, DIRECTION_ANGLES[direction]);
}

// ============================================================================
// SENSOR MEASUREMENTS
// ============================================================================

int read_bus_voltage(void) {
    if (!have_ina219) return 0;
    float v = ina219.getBusVoltage_V();
    return (int)round(1000.0f * v);
}

int read_bus_current(void) {
    if (!have_ina219) return 0;
    float c = ina219.getCurrent_mA();
    return (int)round(c);
}

int read_reverse_power_adc(void) {
    int samples[ADC_AVG_COUNT];
    for (int i = 0; i < ADC_AVG_COUNT; i++) {
        samples[i] = analogReadMilliVolts(REV_POWER_PIN);
        delay(ADC_SAMPLE_DELAY);
    }

    // Partial selection sort — move the ADC_TOP_COUNT largest values to front
    for (int i = 0; i < ADC_TOP_COUNT; i++) {
        for (int j = i + 1; j < ADC_AVG_COUNT; j++) {
            if (samples[j] > samples[i]) {
                int tmp   = samples[i];
                samples[i] = samples[j];
                samples[j] = tmp;
            }
        }
    }

    // Average the top ADC_TOP_COUNT values for a noise-robust peak estimate
    long sum = 0;
    for (int i = 0; i < ADC_TOP_COUNT; i++) sum += samples[i];
    int peak_mv = (int)(sum / ADC_TOP_COUNT);

    DEBUG_PRINTF("Reverse power peak avg: %d mV (top %d of %d)\n",
                 peak_mv, ADC_TOP_COUNT, ADC_AVG_COUNT);
    return peak_mv;
}

void measure_sensors(void) {
    bus_voltage_mv = read_bus_voltage();
    bus_current_ma = read_bus_current();
    DEBUG_PRINTF("Bus: %d mV, %d mA\n", bus_voltage_mv, bus_current_ma);
}

// ============================================================================
// REPLY BUILDING
// ============================================================================

void build_position_reply(int direction) {
    reply_length = 0;

    // Position: ";XYZ"
    reply_buffer[reply_length++] = REPLY_PREFIX_POS;
    reply_buffer[reply_length++] = DIRECTION_ANGLES[direction][0];
    reply_buffer[reply_length++] = DIRECTION_ANGLES[direction][1];
    reply_buffer[reply_length++] = DIRECTION_ANGLES[direction][2];

    // RSSI field: "rRRRR"
    char rssi_str[6];
    sprintf(rssi_str, "r%+04d", (int)radio.getRSSI());
    for (int i = 0; rssi_str[i] != '\0'; i++)
        reply_buffer[reply_length++] = rssi_str[i];

    // Bus voltage: "vVVVVV" (5 digits, millivolts)
    char volt_str[8];
    sprintf(volt_str, "v%05d", bus_voltage_mv);
    for (int i = 0; volt_str[i] != '\0'; i++)
        reply_buffer[reply_length++] = volt_str[i];

    // Bus current: "iIII" (3 digits, milliamps)
    char curr_str[6];
    sprintf(curr_str, "i%03d", bus_current_ma);
    for (int i = 0; curr_str[i] != '\0'; i++)
        reply_buffer[reply_length++] = curr_str[i];

    DEBUG_PRINTF("Position reply length: %d\n", reply_length);
}

void build_power_reply(void) {
    reply_length = 0;

    int adc_mv = read_reverse_power_adc();
    // analogReadMilliVolts() returns pin voltage in mV; scale by voltage divider
    // factor then convert to volts: V_actual = adc_mv * 169.86 / 1000
    float rev_voltage = (adc_mv * REV_POWER_CONVERSION_FACTOR) / 1000.0f;
    float rev_power   = (rev_voltage * rev_voltage) / 100.0f;  // P = Vpeak²/(2×Z0) = Vpeak²/100 (Z0=50 Ω, /2 for RMS)

    reply_buffer[reply_length++] = REPLY_PREFIX_PWR;

    char power_str[8];
    dtostrf(rev_power, 6, 1, power_str);
    for (int i = 0; power_str[i] != '\0' && reply_length < (int)sizeof(reply_buffer); i++)
        reply_buffer[reply_length++] = power_str[i];

    DEBUG_PRINTF("Power reply: %.1f W (ADC: %d mV), length: %d\n",
                 rev_power, adc_mv, reply_length);
}

// ============================================================================
// COMMAND PROCESSING
// ============================================================================

int parse_direction_from_command(void) {
    if (command_length < 7) return current_direction;

    // command_buffer[3..5] = azimuth digits, e.g. "045"
    int digit2 = (int)command_buffer[4] - '0';  // middle digit
    switch (digit2) {
        case 0: return DIR_N;
        case 4: return DIR_NE;
        case 9: return DIR_E;
        case 3: return DIR_SE;
        case 8: return DIR_S;
        case 2: return DIR_SW;
        case 7: return DIR_W;
        case 1: return DIR_NW;
        case 6: return DIR_N;   // 360
        default: return current_direction;
    }
}

void handle_set_direction(int direction) {
    Serial.printf("Setting direction to %d (%s deg)\n",
                  direction, DIRECTION_ANGLES[direction]);

    char terminator = command_buffer[command_length - 1];
    if (terminator == CMD_TERMINATOR_CR) {
        set_antenna_direction(direction);
    } else {
        target_direction = direction;
    }

    measure_sensors();
    build_position_reply(current_direction);
}

void handle_position_query(void) {
    DEBUG_PRINTLN("Position query received");
    measure_sensors();
    build_position_reply(current_direction);
}

void handle_power_query(void) {
    DEBUG_PRINTLN("Power report request received");
    measure_sensors();
    build_power_reply();
}

void process_command(void) {
    if (command_length == 0) return;

    DEBUG_PRINTF("Processing command (%d bytes): %s\n",
                 command_length, command_buffer);

    if (command_length == 1) {
        switch (command_buffer[0]) {
            case CMD_TYPE_POWER:
                handle_power_query();
                break;
            case ';':
                DEBUG_PRINTLN("Stop command");
                measure_sensors();
                build_position_reply(current_direction);
                break;
            default:
                Serial.printf("Unknown single-char command: %c\n", command_buffer[0]);
                break;
        }
        return;
    }

    if (command_length == 3) {
        if (command_buffer[1] == CMD_TYPE_INFO_I) {
            handle_position_query();
        } else if (command_buffer[1] == CMD_TYPE_INFO_M) {
            set_antenna_direction(target_direction);
            measure_sensors();
            build_position_reply(current_direction);
        }
        return;
    }

    if (command_length == 7) {
        if (command_buffer[0] == CMD_PREFIX_A &&
            command_buffer[1] == CMD_PREFIX_P &&
            command_buffer[2] == CMD_PREFIX_1) {
            handle_set_direction(parse_direction_from_command());
        } else {
            Serial.println("Malformed 7-byte command");
        }
        return;
    }

    DEBUG_PRINTF("Unexpected command length: %d\n", command_length);
}

// ============================================================================
// MAIN SETUP AND LOOP
// ============================================================================

void setup() {
    init_all_hardware();
}

void loop() {
    if (!packetReceived) {
        delay(5);
        return;
    }
    packetReceived = false;

    // Read received packet
    size_t pkt_len = radio.getPacketLength();
    if (pkt_len == 0 || pkt_len > (size_t)(MAX_COMMAND_LEN - 1)) {
        pkt_len = MAX_COMMAND_LEN - 1;
    }

    int16_t state = radio.readData((uint8_t*)command_buffer, pkt_len);

    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("ERROR: readData failed, code %d\n", state);
        radio.startReceive();
        return;
    }

    command_length = (int)pkt_len;
    command_buffer[command_length] = '\0';
    packet_count++;

    Serial.printf("Packet #%d [RSSI:%d dBm]: ",
                  packet_count, (int)radio.getRSSI());
    for (int i = 0; i < command_length; i++) Serial.printf("%02X ", (uint8_t)command_buffer[i]);
    Serial.println();

    // Process command and build reply
    reply_length = 0;
    process_command();

    if (reply_length <= 0) {
        Serial.println("No reply to send.");
        radio.startReceive();
        return;
    }

    // Transmit reply (blocking)
    state = radio.transmit(reply_buffer, (size_t)reply_length);
    if (state != RADIOLIB_ERR_NONE) {
        Serial.printf("ERROR: TX reply failed, code %d\n", state);
    } else {
        DEBUG_PRINTF("Reply sent (%d bytes)\n", reply_length);
        digitalWrite(LED, HIGH); delay(10); digitalWrite(LED, LOW);
    }

    // Clear any TX-done interrupt that fired the ISR during transmit,
    // then re-arm receive.
    packetReceived = false;
    radio.startReceive();
}
