# StratoTrack LoRa Pro

**Offline long-range GPS beacon tracker powered by ESP32-C6, NEO-6M GNSS, RAK3172/RAK3272S LoRa P2P, a 2.8-inch SPI TFT dashboard, and a local Wi-Fi web panel.**

StratoTrack LoRa Pro is an embedded tracking project designed for offline field use. The tracker reads GPS/GNSS data from a NEO-6M module, renders a professional live dashboard on an ILI9341 TFT screen, broadcasts compact position packets over LoRa P2P using a RAK3172/RAK3272S module, and exposes a local web dashboard from the ESP32-C6 itself.

The project does **not** require GSM, SIM cards, cloud services, routers, internet access, or LoRaWAN gateways. It is designed as a direct, offline, long-range LoRa beacon system.

---

## Table of Contents

- [Project Goals](#project-goals)
- [What This Project Does](#what-this-project-does)
- [System Architecture](#system-architecture)
- [Hardware Overview](#hardware-overview)
- [Main Tracker Hardware](#main-tracker-hardware)
- [Second LoRa Receiver Hardware](#second-lora-receiver-hardware)
- [Pinout](#pinout)
- [Firmware Structure](#firmware-structure)
- [Programming Language and Libraries](#programming-language-and-libraries)
- [PlatformIO Setup](#platformio-setup)
- [Build, Upload and Monitor](#build-upload-and-monitor)
- [TFT Dashboard](#tft-dashboard)
- [Web Panel](#web-panel)
- [GNSS / Satellite Data](#gnss--satellite-data)
- [LoRa P2P Protocol](#lora-p2p-protocol)
- [Second LoRa Receiver](#second-lora-receiver)
- [Packet Format](#packet-format)
- [Configuration](#configuration)
- [Field Testing Guide](#field-testing-guide)
- [Troubleshooting](#troubleshooting)
- [Safety and Legal Notes](#safety-and-legal-notes)
- [Roadmap](#roadmap)
- [License](#license)

---

## Project Goals

StratoTrack LoRa Pro was built with the following goals:

1. **Offline operation**  
   The device must work without cellular network, internet, cloud platform, or LoRaWAN gateway.

2. **Long-range beaconing**  
   The tracker periodically transmits compact GPS packets over raw LoRa P2P.

3. **Professional field UI**  
   The device has a local TFT interface showing GPS, satellite, LoRa and TX status.

4. **Local diagnostics**  
   A Wi-Fi access point and local web panel allow live monitoring from a phone or laptop.

5. **Modular firmware**  
   The codebase is organized as multiple firmware modules instead of a single sketch.

6. **Expandable architecture**  
   The project can later support ACK messages, LoRa command mode, multiple nodes, track logging, and additional dashboard pages.

---

## What This Project Does

The main tracker device:

- reads NMEA data from a NEO-6M GPS module,
- determines whether GPS state is `WAIT`, `DATA`, or `FIX`,
- parses GPS satellite data from NMEA GSV messages,
- displays GPS and LoRa data on a 2.8-inch SPI TFT,
- starts a local Wi-Fi access point,
- serves a browser-based web dashboard at `http://192.168.4.1`,
- builds a compact tracker packet,
- converts the packet to HEX for RAK RUI3 P2P transmission,
- sends the packet over LoRa P2P,
- prints detailed debug logs through Serial Monitor.

A second LoRa receiver can be added using another ESP32/Arduino-compatible board plus a RAK3172/RAK3272S module. The receiver listens for LoRa P2P packets and decodes the tracker data.

---

## System Architecture

### Main Tracker

```text
NEO-6M GPS
    |
    | UART / NMEA 9600 baud
    v
ESP32-C6
    |
    | SPI
    v
2.8-inch ILI9341 TFT Display

ESP32-C6
    |
    | UART / RAK RUI3 AT Commands
    v
RAK3172 / RAK3272S LoRa Module
    |
    | LoRa P2P 868 MHz
    v
Second LoRa Receiver
```

### Local Web Panel

```text
Phone / Laptop
    |
    | Wi-Fi AP
    v
ESP32-C6 Local Web Server
    |
    v
http://192.168.4.1
```

The web panel is served directly by the ESP32-C6. No router or internet connection is required.

---

## Hardware Overview

### Main Tracker Hardware

Recommended hardware:

| Part | Purpose |
|---|---|
| ESP32-C6 DevKitC-1 | Main controller |
| NEO-6M GPS | GNSS position source |
| RAK3172 / RAK3272S | LoRa P2P modem |
| 2.8-inch SPI TFT ILI9341 | Local dashboard |
| 18650 / LiPo battery system | Portable power |
| USB cable | Programming/debug |
| LoRa antenna | Required before TX |

### Second LoRa Receiver Hardware

For the second LoRa receiver:

| Part | Purpose |
|---|---|
| ESP32 / ESP32-C6 / Arduino-compatible MCU | Receiver controller |
| RAK3172 / RAK3272S | LoRa P2P receiver |
| USB cable | Serial Monitor |
| LoRa antenna | Required |

The second receiver firmware listens in P2P mode and prints decoded packets.

---

## Pinout

The current tested tracker pinout is:

### Main Tracker: ESP32-C6 + RAK + GPS + TFT

| Module | Module Pin | ESP32-C6 Pin | Notes |
|---|---:|---:|---|
| RAK3272S / RAK3172 | UART2_TX | GPIO0 | ESP32 RX for LoRa |
| RAK3272S / RAK3172 | UART2_RX | GPIO1 | ESP32 TX for LoRa |
| RAK3272S / RAK3172 | 3V3 | 3V3 | Do not use 5V |
| RAK3272S / RAK3172 | GND | GND | Common ground |
| NEO-6M | TX | GPIO4 | GPS TX to ESP32 RX |
| NEO-6M | RX | GPIO5 / not required | Optional |
| NEO-6M | VCC | 5V or 3V3 depending on board | Most breakout boards work well with 5V |
| NEO-6M | GND | GND | Common ground |
| TFT ILI9341 | SCK | GPIO6 | SPI clock |
| TFT ILI9341 | MOSI | GPIO7 | SPI MOSI |
| TFT ILI9341 | MISO | Not connected | Not required |
| TFT ILI9341 | CS | GPIO10 | Chip select |
| TFT ILI9341 | DC | GPIO2 | Data/command |
| TFT ILI9341 | RST | GPIO3 | Reset |
| TFT ILI9341 | BL/LED | 3V3 | Backlight always on |

### Important Wiring Rules

- All grounds must be common.
- RAK LoRa module must be powered from 3.3V.
- Do not transmit LoRa without antenna.
- GPS RX is optional; only GPS TX is needed for receiving NMEA.
- If LoRa returns unreadable characters, check baud rate and TX/RX direction.
- If GPS `NMEA` count is zero, the GPS TX pin is not reaching ESP32 RX.

---

## Firmware Structure

StratoTrack LoRa Pro firmware is written in **C++** using the **Arduino framework** on top of **PlatformIO**. The project is intentionally organized as a modular embedded firmware instead of a single-file sketch. Each hardware responsibility is separated into its own module to make the codebase easier to maintain, debug, extend, and publish as an open-source project.

The firmware runs on an **ESP32-C6** main controller and communicates with external modules through UART and SPI interfaces. The GPS module provides GNSS/NMEA data, the RAK LoRa module handles long-range P2P communication through AT commands, the TFT display renders the local dashboard, and the ESP32-C6 exposes a local Wi-Fi web panel.

### `config.h`

Contains all hardware pin definitions, baud rates, LoRa parameters, display settings, firmware metadata and feature flags.

This is the first file to edit when changing:

- pins,
- LoRa frequency,
- spreading factor,
- device ID,
- Wi-Fi SSID/password,
- display rotation,
- beacon interval.

### `globals.h` / `globals.cpp`

Contains shared global objects and shared state variables used across modules.

Examples:

```cpp
TinyGPSPlus gps;
HardwareSerial GPSSerial;
HardwareSerial LoRaSerial;
String gpsState;
String loraState;
String txState;
```

This prevents duplicate object definitions across multiple `.cpp` files.

### `gps_tracker.h` / `gps_tracker.cpp`

Responsible for GPS and GNSS logic:

- reads NMEA data from GPS UART,
- updates TinyGPSPlus,
- tracks NMEA character count,
- detects GPS state: `WAIT`, `DATA`, `FIX`,
- parses GSV satellite messages,
- stores PRN, SNR, elevation and azimuth,
- provides helper functions for latitude, longitude, speed, altitude and satellite count.

### `lora_modem.h` / `lora_modem.cpp`

Responsible for RAK3172/RAK3272S communication:

- sends AT commands,
- configures RAK P2P mode,
- builds LoRa tracker packets,
- converts packets to HEX,
- transmits packets with `AT+PSEND`,
- tracks TX state and last RAK response.

### `display_ui.h` / `display_ui.cpp`

Responsible for the TFT dashboard:

- initializes ILI9341,
- draws the GNSS panel,
- draws LoRa Radio panel,
- draws NMEA Stream panel,
- shows satellite PRN:SNR values,
- shows GPS state, fix, speed and altitude,
- shows LoRa and TX state.

### `web_panel.h` / `web_panel.cpp`

Responsible for the local web dashboard:

- starts ESP32-C6 Wi-Fi AP,
- serves the HTML dashboard,
- exposes `/api/status`,
- shows live GPS/LoRa data,
- shows satellites,
- provides map link when GPS fix is available.

### `main.cpp`

The firmware entry point:

- initializes Serial Monitor,
- initializes TFT,
- initializes GPS UART,
- initializes LoRa UART,
- starts web panel,
- starts LoRa P2P mode,
- runs the main loop,
- updates display,
- handles web requests,
- sends LoRa packets periodically.

---

## Programming Language and Libraries

### Language

```text
C++ / Arduino Framework
```

The project uses Arduino-style APIs, but follows a modular C++ firmware layout.

### Development Environment

```text
VS Code + PlatformIO
```

PlatformIO is recommended because it provides:

- repeatable builds,
- dependency management,
- cleaner modular project structure,
- easier GitHub publishing,
- better Serial Monitor control.

### Main Libraries

#### TinyGPSPlus

```cpp
#include <TinyGPSPlus.h>
```

Used to parse GPS/NMEA data from the NEO-6M module.

Provides:

- latitude,
- longitude,
- speed,
- altitude,
- satellite count,
- GPS fix validity,
- data age.

The project also manually parses `$GPGSV` / `$GNGSV` messages to display satellite PRN, SNR, elevation and azimuth.

#### Adafruit GFX

```cpp
#include <Adafruit_GFX.h>
```

Used for graphics primitives:

- text,
- lines,
- rectangles,
- rounded rectangles,
- colored panels,
- UI layout.

#### Adafruit ILI9341

```cpp
#include <Adafruit_ILI9341.h>
```

Used to drive the 2.8-inch SPI TFT screen.

#### WiFi

```cpp
#include <WiFi.h>
```

Used to create the ESP32-C6 local Wi-Fi access point.

#### WebServer

```cpp
#include <WebServer.h>
```

Used to serve the local dashboard and JSON API.

---

## PlatformIO Setup

Example `platformio.ini`:

```ini
[env:esp32c6]
platform = https://github.com/pioarduino/platform-espressif32.git#develop
board = esp32-c6-devkitc-1
framework = arduino

upload_port = COM3
monitor_port = COM5

monitor_speed = 115200
upload_speed = 460800
monitor_rts = 0
monitor_dtr = 0

lib_deps =
  mikalhart/TinyGPSPlus
  adafruit/Adafruit GFX Library
  adafruit/Adafruit ILI9341
  adafruit/Adafruit BusIO

build_flags =
  -DARDUINO_USB_CDC_ON_BOOT=1
  -DARDUINO_USB_MODE=1
```

### Notes for ESP32-C6

ESP32-C6 Arduino support may require the `pioarduino` platform. If you see:

```text
Error: This board doesn't support arduino framework!
```

use the platform line shown above.

---

## Build, Upload and Monitor

Clean build:

```powershell
pio run -t clean
```

Build:

```powershell
pio run
```

Upload:

```powershell
pio run -t upload
```

Monitor:

```powershell
pio device monitor -p COM5 -b 115200
```

If COM5 disconnects randomly, try:

```ini
monitor_rts = 0
monitor_dtr = 0
```

---

## TFT Dashboard

The TFT dashboard is designed for field usage and shows the most important status without requiring a laptop.

Main sections:

### GNSS Panel

Shows:

- GPS state: `WAIT`, `DATA`, `FIX`,
- used satellites,
- visible satellites,
- mode,
- latitude,
- longitude.

### LoRa Radio Panel

Shows:

- LoRa state,
- TX state,
- last TX status.

### NMEA Stream Panel

Shows:

- NMEA character count,
- GNSS stream activity,
- useful debug status.

### Satellite Panel

Shows:

- GPS PRN numbers,
- SNR values,
- active satellite list.

Example:

```text
G05:24  G12:41  G21:38  G29:15
```

Where:

- `G05` = GPS PRN 05,
- `24` = signal-to-noise ratio.

---

## Web Panel

The ESP32-C6 starts a local Wi-Fi access point.

Default credentials:

```text
SSID: GPS-TRACKER-T1
PASS: 12345678
URL : http://192.168.4.1
```

The web panel displays:

- GPS state,
- latitude and longitude,
- speed,
- altitude,
- satellite count,
- visible satellites,
- LoRa state,
- TX state,
- last LoRa packet,
- RAK response,
- satellite PRN/elevation/azimuth/SNR,
- Google Maps link when GPS fix is available.

### JSON API

The web dashboard reads:

```text
/api/status
```

Example fields:

```json
{
  "device": "T1",
  "firmware": "2.1.0",
  "gpsState": "FIX",
  "fix": true,
  "lat": "39.963205",
  "lon": "32.791445",
  "speed": 1,
  "alt": 1024,
  "sat": 8,
  "gsv": 10,
  "nmea": 12000,
  "lora": "READY",
  "tx": "OK #12",
  "packet": "T1,FIX,39963205,32791445,1,8,1024,10,12,NORMAL"
}
```

---

## GNSS / Satellite Data

The NEO-6M does not provide real satellite names such as official spacecraft names. It provides GPS PRN numbers.

The firmware displays them as:

```text
GPS-05
GPS-12
GPS-21
```

Satellite information parsed from GSV:

| Field | Meaning |
|---|---|
| PRN | Satellite identifier |
| Elevation | Degrees above horizon |
| Azimuth | Direction in degrees |
| SNR | Signal strength / carrier-to-noise value |

### GPS States

| State | Meaning |
|---|---|
| `WAIT` | No NMEA data received |
| `DATA` | NMEA data is coming, but no valid fix |
| `FIX` | Valid position is available |

If NMEA count increases but latitude/longitude are not available, GPS is working but has no fix yet.

---

## LoRa P2P Protocol

StratoTrack uses raw LoRa P2P through RAK RUI3 AT commands.

RAK setup commands:

```text
AT
AT+NWM=0
ATZ
AT
AT+P2P=868100000:12:125:0:8:14
AT+PRECV=0
```

Send command:

```text
AT+PSEND=<HEX_PAYLOAD>
```

The RAK module expects HEX payload for P2P send. Therefore, the firmware converts:

```text
T1,FIX,39963205,32791445,1,8,1024,10,12,NORMAL
```

to HEX before sending.

---

## Packet Format

Current packet format:

```text
ID,STATE,LATx1e6,LONx1e6,SPEED,SAT,ALT,GSV,COUNTER,MODE
```

Example:

```text
T1,FIX,39963205,32791445,1,8,1024,10,12,NORMAL
```

Field details:

| Field | Example | Description |
|---|---:|---|
| ID | `T1` | Device ID |
| STATE | `FIX` | GPS state |
| LATx1e6 | `39963205` | Latitude × 1,000,000 |
| LONx1e6 | `32791445` | Longitude × 1,000,000 |
| SPEED | `1` | Speed in km/h |
| SAT | `8` | Used satellites |
| ALT | `1024` | Altitude in meters |
| GSV | `10` | Visible satellite entries |
| COUNTER | `12` | Packet counter |
| MODE | `NORMAL` | Device mode |

### Example Decoding

```text
39963205 / 1000000 = 39.963205
32791445 / 1000000 = 32.791445
```

Google Maps:

```text
https://maps.google.com/?q=39.963205,32.791445
```

---

## Second LoRa Receiver

A second LoRa receiver can be built using another ESP32-compatible board and RAK3172/RAK3272S module.

The receiver listens in LoRa P2P mode and decodes packets from the tracker.

### Receiver Wiring

| RAK3172 / RAK3272S | Receiver MCU |
|---|---:|
| UART2_TX | GPIO4 |
| UART2_RX | GPIO5 |
| 3V3 | 3V3 |
| GND | GND |

### Receiver LoRa Config

Must match the tracker:

```text
868100000:12:125:0:8:14
```

### Receiver Behavior

The receiver:

- starts RAK in P2P mode,
- enables continuous receive mode,
- waits for `+EVT:RXP2P`,
- extracts HEX payload,
- decodes payload to text,
- parses tracker packet,
- prints coordinates,
- prints Google Maps URL,
- prints RSSI and SNR if available.

Example output:

```text
LORA PACKET RECEIVED
RAW     : T1,FIX,39963205,32791445,1,8,1024,10,12,NORMAL
DEVICE  : T1
STATE   : FIX
LAT     : 39.963205
LON     : 32.791445
MAP     : https://maps.google.com/?q=39.963205,32.791445
SPEED   : 1 km/h
SAT     : 8
ALT     : 1024 m
GSV     : 10
COUNT   : 12
MODE    : NORMAL
RSSI    : -84
SNR     : 10
```

---

## Configuration

Main configuration file:

```text
include/config.h
```

Important settings:

```cpp
#define DEVICE_ID "T1"

#define LORA_FREQ_HZ 868100000UL
#define LORA_SF 12
#define LORA_BW 125
#define LORA_CR 0
#define LORA_PREAMBLE 8
#define LORA_TX_POWER 14

#define BEACON_INTERVAL_NORMAL_MS 10000UL

#define WEB_AP_SSID "GPS-TRACKER-T1"
#define WEB_AP_PASS "12345678"
```

### Beacon Interval

Default:

```cpp
#define BEACON_INTERVAL_NORMAL_MS 10000UL
```

This sends one packet every 10 seconds.

---

## Field Testing Guide

### Step 1: Test TFT

Confirm the TFT shows the dashboard.

### Step 2: Test GPS NMEA

Serial should show:

```text
NMEA increasing
```

If NMEA increases but GPS state is `DATA`, GPS works but has no fix.

### Step 3: Get GPS Fix

Move outdoors or near a clear window.

Expected:

```text
GPS FIX
SAT 5+
LAT valid
LON valid
```

### Step 4: Test LoRa TX

Expected:

```text
RAK: +EVT:TXP2P DONE
TX: T1,FIX,...
```

### Step 5: Test Web Panel

Connect to:

```text
GPS-TRACKER-T1
```

Open:

```text
http://192.168.4.1
```

### Step 6: Test Second Receiver

Power the second receiver and check for `RXP2P` events.

---

## Troubleshooting

### GPS stays WAIT

Cause:

- GPS TX not connected,
- wrong GPS RX pin,
- no common ground,
- GPS not powered,
- wrong baud rate.

Check:

```text
NMEA count = 0
```

This means ESP32 receives no GPS data.

### GPS DATA but no FIX

Cause:

- GPS sees NMEA but no satellite lock.

Fix:

- go outside,
- place antenna facing sky,
- wait 2–5 minutes,
- check satellite count.

### LoRa NO AT

Cause:

- wrong UART pins,
- wrong baud,
- no RAK power,
- no common ground,
- TX/RX swapped.

### LoRa P2P ERR

Cause:

- RAK accepted AT but rejected P2P config,
- RAK may need reboot after `AT+NWM=0`,
- previous mode not applied yet.

Fix:

- use `AT+NWM=0`,
- then `ATZ`,
- wait,
- send `AT`,
- send `AT+P2P=...`.

### LoRa TX works but receiver gets nothing

Check:

- same frequency,
- same SF,
- same bandwidth,
- same coding rate,
- same preamble,
- antennas connected,
- both devices use P2P mode,
- receiver uses `AT+PRECV=65534`.

### Web panel does not open

Check:

- phone connected to `GPS-TRACKER-T1`,
- password is `12345678`,
- open `http://192.168.4.1`,
- disable mobile data if the phone switches away,
- confirm Serial Monitor prints web panel start logs.

### COM5 disconnects

Try:

```ini
monitor_rts = 0
monitor_dtr = 0
```

Use a better USB cable and avoid weak USB hubs.

---

## Safety and Legal Notes

- Always connect a proper antenna before LoRa transmission.
- Do not exceed local RF regulations.
- For EU/Türkiye 868 MHz usage, respect regional duty-cycle and power limits.
- This project is for educational, prototyping and field testing use.
- Do not use for unlawful tracking or surveillance.

---

## Roadmap

Planned improvements:

- ACK packets from second receiver,
- two-way LoRa commands,
- SOS mode,
- adaptive TX interval,
- stopped/moving/fast mode detection,
- local track history,
- CSV export from web panel,
- satellite skyplot on TFT,
- battery voltage monitoring,
- enclosure-ready hardware layout,
- multi-device receiver dashboard.

---

## License

AGPL-3.0 license

---
