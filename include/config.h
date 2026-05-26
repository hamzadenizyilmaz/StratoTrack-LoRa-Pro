#pragma once
#include <Arduino.h>

#define PROJECT_NAME "StratoTrack LoRa Pro"
#define DEVICE_ID "T1"
#define FIRMWARE_NAME "StratoTrack LoRa Pro"
#define FIRMWARE_VERSION "2.2.0"

// DEBUG SERIAL

#define DEBUG_SERIAL_BAUD 115200
#define DEBUG_ENABLED 1

// LORA UART

#define LORA_RX_PIN 0
#define LORA_TX_PIN 1
#define LORA_RST_PIN -1
#define LORA_BAUD 115200

// GPS UART

#define GPS_RX_PIN 4
#define GPS_TX_PIN 5
#define GPS_BAUD 9600

// TFT ILI9341 SPI

#define TFT_SCLK 6
#define TFT_MOSI 7
#define TFT_MISO -1
#define TFT_CS   10
#define TFT_DC   2
#define TFT_RST  3
#define TFT_BL   -1

#define TFT_INVERT_COLORS 0
#define DISPLAY_ROTATION 1


// UART NUMBERS

#define LORA_UART_NUM 0
#define GPS_UART_NUM  1

// LORA P2P SETTINGS
//
// EU868 starter settings.
// Always connect an antenna before transmitting.
//

#define LORA_FREQ_HZ 868100000UL
#define LORA_SF 12
#define LORA_BW 125

// RAK RUI3 coding rate:
// 0 = 4/5
// 1 = 4/6
// 2 = 4/7
// 3 = 4/8
#define LORA_CR 0

#define LORA_PREAMBLE 8
#define LORA_TX_POWER 14

// INTERVALS

#define BEACON_INTERVAL_NORMAL_MS 10000UL
#define BEACON_INTERVAL_FAST_MS   3000UL
#define BEACON_INTERVAL_LONG_MS   30000UL
#define BEACON_INTERVAL_SOS_MS    2000UL

#define SCREEN_INTERVAL_MS 1000UL

#define SEND_NO_FIX_STATUS_PACKET 1

// BATTERY AND BUTTONS

#define BATTERY_ADC_PIN -1
#define BATTERY_DIVIDER_RATIO 2.0f
#define BATTERY_MIN_V 3.30f
#define BATTERY_MAX_V 4.20f

#define BTN_MODE_PIN -1
#define BTN_SOS_PIN  -1

// LOCAL WEB PANEL

#define ENABLE_WEB_PANEL 1
#define WEB_AP_SSID "GPS-TRACKER-T1"
#define WEB_AP_PASS "12345678"

// UI OPTIONS

#define UI_SHOW_SATELLITES 1
#define UI_SHOW_LAST_PACKET 1
#define UI_SHOW_WEB_INFO 1

// PACKET FORMAT
//
// Version 2 CSV payload:
// ID,STATE,LATx1e6,LONx1e6,SPEED,SAT,ALT,GSV,COUNTER,MODE
//
// Example:
// T1,FIX,39963205,32791445,1,8,1024,10,12,NORMAL
//
// STATE:
// WAIT = no live GPS UART data
// DATA = NMEA data received, no valid location fix yet
// FIX  = valid coordinate fix
//

#define PACKET_VERSION 2
