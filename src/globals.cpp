#include "globals.h"

TinyGPSPlus gps;
HardwareSerial GPSSerial(GPS_UART_NUM);
HardwareSerial LoRaSerial(LORA_UART_NUM);

unsigned long lastScreenMs = 0;
unsigned long lastTxMs = 0;
unsigned long lastGpsCharMs = 0;
unsigned long lastRakEventMs = 0;

uint32_t nmeaChars = 0;
uint32_t txCounter = 0;

bool loraReady = false;

String gpsState = "WAIT";
String loraState = "BOOT";
String txState = "NONE";
String modeState = "NORMAL";
String lastPacket = "-";
String lastRakResponse = "-";
String currentNmeaLine = "";
