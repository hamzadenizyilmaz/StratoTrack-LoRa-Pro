# Hardware Guide

## Overview

StratoTrack LoRa Pro uses four main modules:

- ESP32-C6 DevKitC-1
- NEO-6M GPS
- RAK3272S / RAK3172 LoRa
- ILI9341 SPI TFT

## Power

Use common ground for all modules.

Recommended:

- ESP32 from USB
- GPS VCC from ESP32 5V
- RAK from ESP32 3V3
- TFT VCC according to display module requirements

## Important

RAK modules are 3.3V logic devices. Do not connect 5V UART to RAK RX.
