I want to port the project lora_cont_2 from using Adafruit Feather M0 with RFM95 LoRa radio to a Heltec V4.  Here are some details for the porting:

1.  I have placed a default platformio environment in platformio.ini.  It may need some modification, in particular to use the same LoRa libray as the Adafruit version.

2. We will use the pins as defined the variants.h file (part of heltec library)

3. The built in display uses I2C, with pin 17 for SDA and pin 18 for SCL.  These are already defined in the variants.h file.  It display is 128 x 64.  I would like to use it in the portrait orientation.  It uses the SSD1306 driver.

4. We will use the default I2C already defined in variants.h SDA (4) and SCL (3) pins for conntecting to the MCP23017 GPIO expander. 

5.  We will use GPIO3 (ADC1: Channel 2) cofigured with PULLUP to read the PTT status.  It is active low.

6.  I want to be able to use either board.  So we may have to put compile time conditionals for each board.  


