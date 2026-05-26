#pragma once
#include <Arduino.h>

String readRakResponse(uint32_t timeoutMs);
bool sendAT(const String &cmd, const String &expect, uint32_t timeoutMs);

bool beginLoRaUart();
bool initLoRa();
String buildPacket();
bool sendLoRaPacket(const String &packet);
String textToHex(const String &input);
