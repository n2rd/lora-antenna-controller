I want improve the reverse power measurement as the RF power is applied in pulses as short as 24 mS (morse code dit at 50WPM).  Here are some ideas:

1.  Use the calibrated ADC measurement on ESP32-S3 to measure the reverse power more accurately.  On the ESP32S3 this is done using analogReadMilliVolts() instead of analogRead().  This will give a more accurate measurement of the voltage on the reverse power pin, which can be used to calculate the reverse power more accurately.
2. There is a voltage divider and to adjust for this we need to multiply the measured voltage by 169.86. This will be the new calibration factor for the reverse power measurement.
3. Since the power is pulsed, averaging is not helpful.  An yet there is a lot of noise in the measurement. We need to find a statistic to get a better estimate of the peak without the noise.  What would be a good method?

One approach is the following:
Make 10 measurements, 10mS apart.  Take the average of just the top 3 values.  This will give us a better estimate of the peak power while reducing the impact of noise.  The 10mS interval is chosen to capture the short pulses of RF power, and averaging the top 3 measurements helps to mitigate any outliers caused by noise.

Any other ideas?