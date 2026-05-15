# 📱 Flutter Controller App

A custom Android Bluetooth controller app for the Pick & Place robot. Built with Flutter for a clean, responsive UI optimized for landscape use during competition.

---

## Features

- **One-tap Bluetooth connection** — scans paired devices, connects to HC-05 in seconds
- **Drive controls** — forward, backward, sharp turns, curves while moving
- **Hold-to-move** — press and hold turns, curves, and arm buttons for continuous movement; release to stop
- **Speed presets** — START / DRIVE / RACE / MAX (no slider, fast to tap during race)
- **Full arm control** — gripper, elbow, base with tap (nudge) or hold (continuous)
- **One-button sequences** — PICK, PLACE, U-TURN, CANCEL
- **Scrollable panels** — all controls accessible on any screen size
- **Landscape only** — optimized for two-handed competition use

---

## Screenshots

```
┌─────────────────────────────────────────────────────┐
│ [CONNECT] ● Connected   [START][DRIVE][RACE][MAX]   │
├──────────────────────┬──────────────────────────────┤
│  // DRIVE            │  // ARM                      │
│  [▲ FWD]             │  GRIPPER                     │
│  [◄][STOP][►]        │  [OPEN][CLOSE]               │
│  [↰CURVE][CURVE↱]    │  ELBOW                       │
│  [▼ BWD]             │  [ARM▲][ARM▼]                │
│  [AUTO][MANUAL]      │  BASE                        │
│                      │  [BASE◄][BASE►]              │
│                      │  PRESETS                     │
│                      │  [ARM HOME][ARM PICK]        │
│                      │  SEQUENCES                   │
│                      │  [🤏PICK][📦PLACE]           │
│                      │  [↩U-TURN][✕CANCEL]          │
└──────────────────────┴──────────────────────────────┘
```

---

## Requirements

- Flutter SDK 3.0+
- Android 5.0+ (API 21+)
- Android phone with Bluetooth
- HC-05 paired in Android Bluetooth settings before opening app

---

## Building the APK

```bash
# Install dependencies
flutter pub get

# Build release APK
flutter build apk --release

# APK location
build/app/outputs/flutter-apk/app-release.apk
```

Transfer the APK to your Android device and install it. You'll need to allow **Install from unknown sources** when prompted.

---

## First Time Setup

1. On your Android phone, go to **Settings → Bluetooth**
2. Pair with **HC-05** (default PIN is usually `1234` or `0000`)
3. Open the app
4. Tap **CONNECT** → select HC-05 from the list
5. Tap **MANUAL** to enable manual control

---

## Button Reference

### Drive Panel (Left)

| Button | Sends | Behaviour |
|--------|-------|-----------|
| ▲ FWD | `F` | Tap to go forward (obstacle-aware) |
| ◄ | `L` | Hold = spin left, release = stop |
| STOP | `S` | Tap to stop |
| ► | `R` | Hold = spin right, release = stop |
| ↰ CURVE | `l` | Hold = curve left while moving, release = stop |
| CURVE ↱ | `r` | Hold = curve right while moving, release = stop |
| ▼ BWD | `B` | Tap to go backward |
| AUTO | `A` | Switch to auto/line-following mode |
| MANUAL | `M` | Switch to manual Bluetooth control |

### Speed Presets (Top Bar)

| Button | Sends | PWM |
|--------|-------|-----|
| START | `3` | 75 — minimum movement speed |
| DRIVE | `5` | 125 — normal navigation |
| RACE | `8` | 200 — fast movement |
| MAX | `0` | 255 — full speed |

### Arm Panel (Right, scrollable)

| Button | Sends | Behaviour |
|--------|-------|-----------|
| OPEN | `O` | Gripper fully open |
| CLOSE | `C` | Gripper fully close |
| ARM ▲ | `u` | Hold = elbow moves up continuously |
| ARM ▼ | `d` | Hold = elbow moves down continuously |
| BASE ◄ | `q` | Hold = base rotates left |
| BASE ► | `e` | Hold = base rotates right |
| ARM HOME | `U` | Elbow jumps to home (0°) |
| ARM PICK | `D` | Elbow jumps to pick position (50°) |
| 🤏 PICK | `P` | Full pick sequence (lower → grip → raise) |
| 📦 PLACE | `X` | Full place sequence (lower → open → raise) |
| ↩ U-TURN | `T` | Robot spins 180° then stops |
| ✕ CANCEL | `K` | Cancels any running sequence immediately |

---

## Project Structure

```
flutter_app/
├── lib/
│   └── main.dart          # Entire app (single file)
├── android/
│   ├── app/
│   │   ├── build.gradle   # App-level Gradle config
│   │   └── src/main/
│   │       ├── AndroidManifest.xml
│   │       └── kotlin/.../MainActivity.kt
│   ├── build.gradle       # Root Gradle
│   └── settings.gradle    # Declarative plugin config
└── pubspec.yaml           # Dependencies
```

---

## Dependencies

```yaml
flutter_bluetooth_serial: ^0.4.0  # Classic Bluetooth (HC-05/HC-06)
permission_handler: ^11.0.0       # Runtime Bluetooth permissions
```

> **Note:** `flutter_bluetooth_serial` is overridden via git to fix namespace compatibility with AGP 8.6+. See `pubspec.yaml` for the override.

---

## Known Issues & Notes

- **iOS not supported** — `flutter_bluetooth_serial` only works on Android. HC-05 uses Classic Bluetooth which iOS blocks entirely.
- **macOS 26 beta** — CH340 driver incompatible, use a Windows machine or older macOS to upload Arduino code.
- The app **locks to landscape** orientation via `AndroidManifest.xml`.
- Bluetooth must be **paired** (not just nearby) before connecting through the app.
