
IOT BATHROOM ALERT SYSTEM

An Arduino-based alert system designed to optimize facility maintenance. 
When a service button is pressed, the hardware device broadcasts a fast, fire-and-forget UDP
alert package over Wi-Fi to a central monitoring computer running a background monitoring framework,
notifying staff exactly which bathroom requires immediate sanitation or attention.

Hardware Requirements

Microcontroller: Arduino UNO R4 WiFi (or any standard WiFiS3 compatible board)
Display: SSD1306 128x64 OLED Display (connected via Qwiic Port / Hardware I2C on Wire1)
Input: 1x Physical Momentary Push Button
Wiring:
	 Button connected to Digital Pin 4 and GND (utilizes internal INPUT_PULLUP).
	 OLED Screen connected to the dedicated I2C bus (0x3C default address).
