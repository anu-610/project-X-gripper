# Project X - Gripper

ESP32 controlled gripper bot.

Main electronic componets Used: 

+ ESP32 Development Board

+ L293D Motor Driver IC

+ 4xDC motor

+ MG996r servo
+ 2x MG90 servo

+ HC-SR04 Ultrasonic Sensor (For knowing the distance when it is inside the tunnel)
<br>
The whole thing is controlled wirelessly from RemoteXY app from phone using Bluetooth LE. <br>

## How to Use the App (RemoteXY)

The controller app is built using RemoteXY.<br>

+ Download the "RemoteXY" app from the Google Play Store.


+ Open the app
+ Click on add device and select Bluetooth
+ The app will scan for devices. You should see one named "Project X".

+ Tap it to connect. The controller interface will load, and you're ready to drive!

## Circuit Design

![Circuit Diagram](https://github.com/anu-610/project-X-gripper/blob/main/circuit_image.png)

## RemoteXY GUI
![RemoteXY GUI](https://github.com/anu-610/project-X-gripper/blob/main/gui.png)

