## TicTalk: A multilingual word clock using an LED Matrix

**TicTalk** is an ESP32-powered word clock that displays the time as natural, informal sentences across an LED matrix. Designed with a touch-sensitive interface, automated network time syncing, and a simple configuration portal, TicTalk turns timekeeping into an interactive piece of ambient functional art.

---

## Key Features

- **Informal Word-Based Time:** Spells out the time using natural, conversational phrases rather than raw numbers.
- **Multilingual Support:** Switch seamlessly between **English**, **Spanish**, **French**, and **German**.
- **Dynamic Color Coding:** Words automatically shift color palettes based on the time of day.
- **Touch Interface:** Tap to toggle languages on the fly; perform a **long touch** to launch the setup mode.
- **Automated Timekeeping:** Features daily auto-synchronization using official NTP time servers to stay accurate down to the second.
- **Wi-Fi & Timezone Portal:** Embedded captive portal (`WiFiManager`) for easy Wi-Fi setup, regional timezone selection with automatic DST handling, and a manual offset override option.
- **Non-Volatile Storage:** Saves your network credentials, selected language, and timezone settings across reboots using ESP32 `Preferences`.

## Sample Displays

<div style="display: flex; flex-wrap: wrap; gap: 10px; justify-content: center;">
  <img src="https://morinricardo.com/images/uploads/wordclock-spanish.jpg" width="400" alt="Spanish Panel">
  <img src="https://morinricardo.com/images/uploads/wordclock-english.jpg" width="400" alt="English Panel">
  <img src="https://morinricardo.com/images/uploads/wordclock-french.jpg" width="400" alt="French Panel">
  <img src="https://morinricardo.com/images/uploads/wordclock-german.jpg" width="400" alt="German Panel">
</div>

## Hardware Components

TicTalk uses a modular component stack designed to eliminate messy wiring by plugging directly into the display panel.

| Component | Description |
| --- | --- |
| HUB75 RGB LED Matrix Panel (64x64) | High-density 64x64 dot matrix panel providing the full canvas for rendering multilingual word layouts. |
| ESP32-DevKitC-32E Board | Dual-core microcontroller managing DMA display memory, Wi-Fi networking, touch inputs, and background NTP sync tasks. |
| ElectroDragon RGB Matrix Panel Drive Interface Board for ESP32 DMA | Dedicated shield that plugs directly into the HUB75 header for a simple, cable-free connection, driving flicker-free parallel DMA display updates. Jumper E-18 configured to enable 64x64; Miniswitch set to DEVKitC. |

## Physical Assembly

TicTalk is built as a compact, self-contained unit designed around the LED matrix panel.

- **Front Face & Core:** The 64x64 HUB75 LED matrix panel serves as the structural front face of the clock. The ESP32 development board and DMA adapter board plug directly into the back header of the panel for a cable-free connection.
- **Enclosure & Mounting:** A custom 3D-printed back panel mounts directly to the rear mounting screw points on the LED matrix. The assembly is secured using three nylon standoffs and one brass standoff.
- **Touch Interface:** The brass standoff is electrically connected to the tactile input pin of the ESP32, serving as a reliable capacitive touch interface accessible through the enclosure and the only metal screw in the back of the clock.
- **Power & Control:** The back panel houses a DC 5.5 x 2.1 mm female power socket wired through a Mini Inline Fuse Holder (1A) for circuit protection, alongside a 12mm latching push-button switch with a raised head for power control. All internal wiring and hardware components are cleanly enclosed within the 3D-printed back housing.

## Software Architecture

TicTalk's firmware is structured around a modular architecture using the ESP32 platform within the Arduino framework, leveraging FreeRTOS multitasking to separate display rendering, touch input, and network time synchronization.

### File Structure & Modules

- **`WordClockLED.ino`**: The primary entry point. Initializes the HUB75 DMA display, loads saved language preferences from Non-Volatile Storage (NVS), initiates time synchronization, and starts the FreeRTOS background tasks. It also hosts the main loop managing configuration portal triggers and nightly 3:00 AM NTP resync timers.
- **`ClockTask.h / .cpp`**: Runs as a dedicated FreeRTOS background task pinned to a core. It periodically checks local time and handles time-of-day normalized color blending (transitioning from White at 6 AM through Orange and Light Blue to Dark Blue at midnight). It monitors pause states and language changes, updating and rendering the matrix display efficiently only when the hour, minute, or language preference changes.
- **`DisplayTime.h / .cpp`**: Handles word-to-time mapping, string layout calculation, and dynamic rendering on the 64x64 matrix. It includes custom pixel drawing routines for accented French characters (`É`, `È`, `Ô`) and computes dynamic row centering and variable-width spacing.
- **`TimeWords.h / .cpp`**: Manages localization, structural grammar rules, and text string generation for Spanish, English, French, and German word-clock layouts. It handles 12-hour/24-hour conversions, minute-fraction modifiers (such as "after", "to", "past", and precise quarter/half offsets), hour-flip offsets, and specialized linguistic adjustments like French midday/midnight exceptions and German grammatical variations.
- **`constants.h`**: Lightweight header defining global language constants (`SPANISH`, `ENGLISH`, `FRENCH`, `GERMAN`).
- **`TouchTask.h / .cpp`**: Runs as a separate FreeRTOS background task pinned to Core 1, continuously polling capacitive pin 32. It distinguishes between a short tap (< 800ms) to cycle through languages (Spanish, English, French, and German) with state persistence via NVS preferences, and a long hold (> 3s) to safely trigger the Wi-Fi provisioning and timezone configuration portal flag (`triggerPortal`).
- **`TimeSync.h / .cpp`**: Manages network connectivity, automated NTP synchronization via `configTzTime` using custom POSIX timezone strings, and background resync routines. It integrates `WiFiManager` to host a captive portal (`TicTalk-XXXX`) featuring an injected configuration web UI for selecting global timezones or setting custom numeric offsets, saving parameters securely to NVS preferences and driving visual feedback and spinner animations directly on the LED matrix during connection states.

### Core Libraries & Dependencies

| Library | Purpose |
| :--- | :--- |
| **ESP32 HUB75 LED MATRIX PANEL DMA Display** | High-performance parallel DMA display driver for flicker-free rendering. |
| **Adafruit GFX Library** | Core graphics primitives and text layout engine. |
| **WiFiManager** | Captive portal engine for runtime Wi-Fi credential and parameter setup. |
| **Preferences** | Native built-in ESP32 core library for non-volatile storage (NVS). |

### Execution Flow

1. **Boot & Initialization**: Serial logging starts, the HUB75 matrix is initialized via I2S DMA, and saved preferences are loaded from NVS.
2. **Time Sync & Network**: `TimeSync` attempts to connect using stored credentials and synchronize with NTP servers. If credentials are missing or the touch portal trigger is activated, it launches the captive portal Access Point (`TicTalk-XXXX`).
3. **Task Scheduling & Rendering**: `ClockTask` runs in the background to fetch time, passing hours and minutes to `DisplayTime` to format, wrap, and render strings onto the display matrix. `TouchTask` concurrently monitors for user input.
4. **Routine Maintenance**: A background check runs every second in the main loop to execute an automatic NTP resync daily at 3:00 AM.

## Building and Flashing

### Prerequisites

1. **Arduino IDE**: Install version 2.x.
2. **ESP32 Board Support**: Ensure the **esp32 by Espressif Systems** core package (v3.3.8 or compatible) is installed via the Arduino Boards Manager.
3. **Required Libraries**: Install the following dependencies via the Arduino Library Manager:
   * `ESP32-HUB75-MatrixPanel-I2S-DMA` (by MrCodetastic, v3.0.14 or compatible)
   * `Adafruit GFX Library` (v1.12.6 or compatible)
   * `WiFiManager` (by tzapu, v2.0.17 or compatible)

### Board Settings

- **Board**: Select `ESP32 Dev Module` from your board list. Default partition and flash settings.

### Flashing Steps

1. Clone or download this repository to your local machine.
2. Open the project folder and launch `WordClockLED.ino` in the Arduino IDE.
3. Connect your ESP32 to your computer via USB.
4. Select the correct COM port and board target in the IDE tools menu.
5. Click **Upload** to compile and flash the firmware.

### First-Time Setup

1. Upon first boot, if no Wi-Fi credentials are saved, the clock will launch a configuration portal access point named `TicTalk-XXXX`.
2. Connect to this Wi-Fi network using a smartphone or computer.
3. Select your local network, enter your credentials, and choose your local timezone from the dropdown list or input a custom numeric offset.
4. Save the configuration. The clock will automatically reboot and synchronize time via NTP.

---
*This project was developed with the assistance of Google Gemini.*