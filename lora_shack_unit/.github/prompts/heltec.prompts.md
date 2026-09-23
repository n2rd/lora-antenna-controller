Hello Claude.  I think rather than trying to get the LoRa communication code to work with both Adafruit and Heltec, I have decided to separate the two.  I created a new project lora_cont_heltec with the code from our previous attempt.

First, we can clean this code up and remove the Adafruit option completely.  (I have saved that separately in another folder).  

Second, I have done something similar on the phaser side by creating a new project lora_phaser_heltec.  This is just copied over from the adafruit version and will need to be modified.

Third, the key is that I want reliable communication betwen the controller and the phaser.  What are our options with the preferred LoRa library for Heltec, the sandeep mistry library?

By having the phaser and controller side by side we can make appropriate changes to both to get the communication going.

Lets plan this change.

Thanks.