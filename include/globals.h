#pragma once
#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "config.h"

extern TinyGPSPlus gps;
extern HardwareSerial GPSSerial;
extern HardwareSerial LoRaSerial;

extern unsigned long lastScreenMs;
extern unsigned long lastTxMs;
extern unsigned long lastGpsCharMs;
extern unsigned long lastRakEventMs;

extern uint32_t nmeaChars;
extern uint32_t txCounter;

extern bool loraReady;

extern String gpsState;
extern String loraState;
extern String txState;
extern String modeState;
extern String lastPacket;
extern String lastRakResponse;
extern String currentNmeaLine;
