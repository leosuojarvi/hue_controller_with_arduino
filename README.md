# hue_controller_with_arduino
Philips Hue light controller using ESP8226 board with Arduino Uno

This project uses Arduino Uno, ESP8266 board, HC-SR04 ultrasonic sensor and a joystick to control Philips Hue light brightness.

Load hue_controller_esp8266.ino to the memory of your esp8266 and hue_controller_UNO.ino to the memory of your Arduino UNO.

You will have to modify 5 parameters in the file hue_controller_esp8266 to use it. These are ip, (IP of your HUE gateway), user_name (your Hue api user, you can create one with these instructions https://developers.meethue.com/develop/get-started-2/), ssid (your wifi network name in string format, e.g. "My WIFI"), password (your wifi password) and lamps (the id:s of the lamps you want to use, you can find the id:s with the information in the webpage linked previously.

The physical wire connections can be made differenty, but then you will have to modify the code. I have a board with Arduino Uno and ESP8266 combined, but you can connect them with RX and TX pins. My other connections are as follows: 

Joystick:
GND - Arduino Uno ground
Vcc - Arduino Uno 5V
X-axis movement - A0
Y-axis movement - A1
Switch - D2

SR-HC04
GND - Arduino ground
Vcc - Arduino 5V
Trig - Arduino D12
Echo - Arduino D11

Usage:

To use the controller, you can use the joystick: Stick to left changes to previous lamp, stick to right next lamp. Clicking switches the lamp to maximum brightness if it was off and shuts it down if it already was on.
You can also use the ultrasonic sensor (obiously cooler): place the sensor directing upwards and then move your hand up and down. The selected lamp will change the brightness accordingly. By holding your hand steady for a while, you can control all the lamps at the same time until you take tour hand off.



