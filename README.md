# In-Pipe-Inspection-Robot
Wi-Fi controlled in-pipe inspection robot using ESP32, ESP32-CAM and MIT App Inventor
## Features
- Forward, backward and stop control from a smartphone over Wi-Fi
- Live video from a separate ESP32-CAM board
- Distance measurement with an HC-SR04 ultrasonic sensor
- Obstacle detection with an IR sensor
- 12 V LED inspection light switched from the app through a MOSFET
- Motors stop automatically if the robot loses Wi-Fi
- Runs from a battery pack, so no laptop is needed while operating
## How it works
Smartphone app (MIT App Inventor)
        |  HTTP requests over the phone hotspot (2.4 GHz Wi-Fi)
        +--> ESP32 DevKit : motors, HC-SR04, IR sensor, LED
        +--> ESP32-CAM    : live video stream 
        
The two ESP32 boards run separate programs. They share only the Wi-Fi network, so each has its own IP address.
## Hardware
- ### Part : Purpose
- ESP32 DevKit	Main controller : motors, sensors, LED, web server
- ESP32-CAM (AI-Thinker) Camera and video stream
- L298N dual H-bridge	Drives the four DC gear motors
- 4 x TT DC gear motors	Two parallel pairs, one pair per L298N channel
- HC-SR04	Distance measurement
- IR obstacle sensor	Near / far detection
- 12 V LED strip (about 1.5 W)	Inspection light
- IRLZ44N MOSFET	Switches the LED strip (220 ohm gate resistor, 10 kohm pull-down)
- 3 x 18650 Li-ion cells (3S) with 3S BMS	Power source, about 11.1 V nominal
- Buck converter	Reduces the battery voltage to 5 V for the ESP32 boards
## Pin connections (main ESP32)
- Function : ESP32 GPIO
- L298N ENA :	25
- L298N IN1	: 26
- L298N IN2	: 27
- L298N ENB	: 14
- L298N IN3	: 32
- L298N IN4	: 33
- HC-SR04 TRIG : 18
- HC-SR04 ECHO	: 19
- IR sensor output :	23
- LED MOSFET gate :	13
## Wiring notes:
- All grounds (battery, buck converter, ESP32 boards, L298N, sensors, MOSFET) must be joined.
- ESP32 pins are 3.3 V only. Reduce the HC-SR04 ECHO signal with a voltage divider (for example 1 kohm in series and 2 kohm to ground), and check the IR sensor output voltage before connecting it.
- Never power the motors or the LED strip from an ESP32 pin.
- The ESP32-CAM needs a stable 5 V supply. A 100 to 470 uF capacitor close to its 5 V and GND pins helps prevent resets.
## Repository layout
firmware/
  main_esp32/main_esp32.ino   Motors, sensors, LED and web server
  esp32_cam/esp32_cam.ino     Camera stream
app/
  PipeInspectionRobot.aia     MIT App Inventor project
docs/                         Report, presentation, diagrams
images/                       Photos and screenshots

## Setup
1. ### Main ESP32 firmware
   1.Install the Arduino IDE and add the ESP32 boards package (Espressif). Use version 3.x, because the code uses analogWrite.
   2.Open firmware/main_esp32/main_esp32.ino.
   3.Set ssid and password to your Wi-Fi hotspot. Use a 2.4 GHz network.
   4.Select the board ESP32 Dev Module and upload.
   5.Open the Serial Monitor at 115200 baud and note the IP address. 
2. ### ESP32-CAM firmware
   1.Open firmware/esp32_cam/esp32_cam.ino and set the same Wi-Fi details.
   2.Select the board AI Thinker ESP32-CAM and upload.
   3.Note the IP address from the Serial Monitor. The stream is at http://<camera-ip>:81/stream.
3. ### Mobile app
   1.Go to MIT App Inventor and choose Projects, then Import project (.aia).
   2.Select app/PipeInspectionRobot.aia.
   3.Change the IP addresses in the app to the two addresses from the Serial Monitor.
   4.Connect your phone to the same hotspot and run the app.
## HTTP endpoints (main ESP32, port 80)
- ### Endpoint	: Action
- / : Connection test page
- /forward :	Drive forward
- /backward	: Drive backward
- /stop :	Stop the motors
- /distance :	Distance in cm, or NO ECHO
- /ir :	NEAR or CLEAR
- /status :	Vehicle, LED and IR status
- /led/on , /led/off :	Switch the inspection light
- /led/status	: ON or OFF
