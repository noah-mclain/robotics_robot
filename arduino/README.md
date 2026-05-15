# 🔧 Arduino Firmware — robot_controller.ino

## Overview

This is the Arduino Uno firmware for the Pick & Place robot. It handles:
- Bluetooth command parsing (HC-05)
- Differential drive motor control
- 3-DOF robotic arm control via PCA9685
- Non-blocking ultrasonic obstacle detection
- Line tracking for auto mode
- Preset arm sequences (pick, place, U-turn)

---

## Dependencies

Install these libraries via Arduino IDE → Tools → Manage Libraries:

| Library | Version | Purpose |
|---------|---------|---------|
| `Adafruit PWM Servo Driver` | Latest | PCA9685 servo control |
| `SoftwareSerial` | Built-in | Bluetooth UART |
| `Wire` | Built-in | I2C for PCA9685 |

---

## Bluetooth Command Reference

All commands are single ASCII characters sent over Bluetooth at **9600 baud**.

### Mode Control
| Command | Action |
|---------|--------|
| `A` | Auto mode (line tracking + obstacle avoidance) |
| `M` | Manual mode (Bluetooth control) |

### Driving (Manual mode only)
| Command | Action |
|---------|--------|
| `F` | Forward (obstacle-aware) |
| `B` | Backward |
| `L` | Sharp turn left (spin in place) |
| `R` | Sharp turn right (spin in place) |
| `l` | Curve left while moving forward |
| `r` | Curve right while moving forward |
| `S` | Stop |

### Speed (1 = slowest, 0 = max)
| Command | PWM Value |
|---------|-----------|
| `1` | 25 |
| `2` | 50 |
| `3` | 75 (minimum movement speed) |
| `4` | 100 |
| `5` | 125 |
| `6` | 150 |
| `7` | 175 |
| `8` | 200 |
| `9` | 225 |
| `0` | 255 (max) |

### Gripper (CH 0)
| Command | Action |
|---------|--------|
| `O` | Fully open (0°) |
| `C` | Fully close (80°) |
| `+` | Close 2° |
| `-` | Open 2° |

### Elbow (CH 2)
| Command | Action |
|---------|--------|
| `U` | Home position (0°) |
| `D` | Pick position (50°) |
| `u` | Nudge up 2° |
| `d` | Nudge down 2° |

### Base (CH 1)
| Command | Action |
|---------|--------|
| `Q` | Home position (0°) |
| `E` | Max position (45°) |
| `q` | Rotate left 2° |
| `e` | Rotate right 2° |

### Sequences (automated multi-step actions)
| Command | Sequence |
|---------|---------|
| `P` | **PICK** → lower elbow (800ms) → close gripper → raise elbow |
| `X` | **PLACE** → lower elbow (600ms) → open gripper → raise elbow |
| `T` | **U-TURN** → spin left for 450ms → stop |
| `K` | **CANCEL** any running sequence |

---

## Key Design Decisions

### Non-blocking architecture
Everything runs without `delay()` — the loop runs thousands of times per second:
- **Elbow** moves 1° every 20ms using `millis()` timer
- **Ultrasonic** fires every 80ms using a state machine
- **Sequences** use a state machine with `millis()` timers

### Obstacle detection in manual mode
The `goingForward` flag means while `F` is active, the robot re-checks distance every loop:
- **>30cm** → full speed
- **15–30cm** → 50% speed (warning zone)
- **<15cm** → stops forward movement (can still reverse/turn)

### Curve vs Turn
- `L`/`R` = spin in place (one wheel forward, one backward)
- `l`/`r` = curve while moving (both wheels forward, one at 1/3 speed)

---

## Tuning Guide

| Parameter | Location | Effect |
|-----------|----------|--------|
| `stopDistance` | Line 47 | Distance (cm) to hard stop |
| `warnDistance` | Line 48 | Distance (cm) to slow down |
| `ultraInterval` | Line 52 | How often ultrasonic fires (ms) |
| `PICK_WAIT` timeout | `updateSequence()` | How long elbow lowers before grip |
| `PLACE_WAIT` timeout | `updateSequence()` | How long elbow lowers before open |
| U-turn duration | `UTURN_SPIN` case | Spin time for 180° (speed-dependent) |
| Curve ratio | `curveLeft/Right()` | `speedValue / 3` — lower = sharper |

---

## Uploading

1. Open `robot_controller.ino` in Arduino IDE 2
2. Select **Tools → Board → Arduino Uno**
3. Select the correct port under **Tools → Port**
4. Click **Upload** (→ arrow)

> **Note for Mac users:** If your Arduino uses a CH340 chip (common on clones), you need the [CH34x driver](https://www.wch.cn/downloads/CH34XSER_MAC_ZIP.html). macOS 26 beta has known compatibility issues — use a Windows machine as a workaround.
