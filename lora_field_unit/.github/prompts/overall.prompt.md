I have a pair of related projects: lora_phaser_heltec and lora_cont_heltec.  They communicate with each other using LoRa.  

The lora_cont_heltec project is the controller.  It accepts input from the buttons and USB serial port and sends commands using lora to lora_phaser_heltec ih the field.

The lora_phaser_heltec processes the commands and sends a packet of information back to the controller.
