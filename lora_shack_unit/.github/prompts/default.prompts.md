When the unit boots up, it should default to DIR_NE and that LED should be lit.  Instead, it seems to go an random state (different one each time).This is likely an initialization error that needs to be fixed.

After a button is pressed, the controller waits for a reply and then uses the direction in the apply to do the following:

1. It should blink that LED that corresponds to the reply twice and then leaves it lit. (0.5 sec cycle for blinking)
2. It should show the direction in the reply on the LED.

