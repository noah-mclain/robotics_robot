# 🤖 Autonomous Mobile Robot Pick & Place — Controller System

**Arab Academy for Science, Technology & Maritime Transport — Smart Village**  
**Course:** Robotics Applications (CCS4605) | **Instructor:** Dr. Magdy Naeem  
**Team:** Malak Maher · Nada Ayman · Rola Khaled · Zeina Ahmed

---

## Project Overview

This repository contains the full control system for our autonomous mobile robot, built for the Pick & Place Competition. The robot navigates an arena, picks up blocks (1×1, 2×2, 3×3 cm), and places them in sorted areas — all controlled via a custom Bluetooth Android app.

The system is split into two parts:

| Folder | Description |
|--------|-------------|
| [`arduino/`](./arduino/) | Arduino Uno firmware — motors, servos, sensors |
| [`flutter_app/`](./flutter_app/) | Android controller app built with Flutter |

---

## System Architecture

```
Samsung Android Phone
        │
        │  Bluetooth (HC-05, Classic BT)
        ▼
   Arduino Uno
   ┌──────────────────────────────┐
   │  Motor Driver (H-Bridge)     │ → 2× DC Motors (differential drive)
   │  PCA9685 PWM Driver          │ → 3× Servo Motors (arm + gripper)
   │  Ultrasonic Sensor (HC-SR04) │ → Obstacle detection
   │  3-Channel Line Tracker      │ → Auto mode navigation
   └──────────────────────────────┘
```

---

## Hardware

| Component | Spec |
|-----------|------|
| Microcontroller | Arduino Uno |
| Drive | 2× DC geared motors, differential drive |
| Arm | 3 DOF, 3× SG90 servo motors |
| Gripper | Servo-actuated parallel gripper |
| Sensors | HC-SR04 ultrasonic + 3-channel line tracker |
| Bluetooth | HC-05 on SoftwareSerial pins 2 (RX) & 3 (TX) |
| Servo driver | PCA9685 16-channel PWM (I2C) |
| Power | 3× 3.7V Li-ion batteries |

---

## Pin Mapping

| Pin | Function |
|-----|----------|
| 2 | BT RX (SoftwareSerial) |
| 3 | BT TX (SoftwareSerial) |
| 5 | ENA (Motor A speed) |
| 6 | ENB (Motor B speed) |
| 7 | IN1 |
| 8 | IN2 |
| 9 | IN3 |
| 10 | IN4 |
| 11 | Ultrasonic TRIG |
| 12 | Ultrasonic ECHO |
| A0 | Line tracker LEFT |
| A1 | Line tracker MIDDLE |
| A2 | Line tracker RIGHT |
| SDA/SCL | PCA9685 I2C |

---

## Quick Start

1. Upload [`arduino/robot_controller.ino`](./arduino/robot_controller.ino) to the Arduino Uno
2. Build and install the Flutter app — see [`flutter_app/README.md`](./flutter_app/README.md)
3. Power on the robot
4. Pair your Android phone with **HC-05** via Bluetooth settings
5. Open the app → tap **CONNECT** → select HC-05
6. Tap **MANUAL** to start driving

---

## License

Open source for educational use. Feel free to use and adapt with attribution.
