#include "lora_modem.h"

#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "gps_tracker.h"
#include "display_ui.h"

bool beginLoRaUart() {
  LoRaSerial.begin(LORA_BAUD, SERIAL_8N1, LORA_RX_PIN, LORA_TX_PIN);
  Serial.println("LORA UART OK");
  return true;
}

String readRakResponse(uint32_t timeoutMs) {
  String response;
  uint32_t start = millis();

  while (millis() - start < timeoutMs) {
    while (LoRaSerial.available()) {
      char c = (char)LoRaSerial.read();
      response += c;
    }

    delay(2);
  }

  response.trim();

  if (response.length() > 0) {
    lastRakResponse = response;
    lastRakEventMs = millis();
  }

  return response;
}

bool sendAT(const String &cmd, const String &expect, uint32_t timeoutMs) {
  while (LoRaSerial.available()) {
    LoRaSerial.read();
  }

  Serial.print("RAK CMD: ");
  Serial.println(cmd);

  LoRaSerial.print(cmd);
  LoRaSerial.print("\r\n");

  String response = readRakResponse(timeoutMs);

  if (response.length() > 0) {
    Serial.print("RAK: ");
    Serial.println(response);
  } else {
    Serial.println("RAK: <EMPTY>");
  }

  if (expect.length() == 0) {
    return response.length() > 0;
  }

  return response.indexOf(expect) >= 0;
}

String textToHex(const String &input) {
  const char hex[] = "0123456789ABCDEF";
  String out;
  out.reserve(input.length() * 2);

  for (size_t i = 0; i < input.length(); i++) {
    uint8_t c = (uint8_t)input[i];
    out += hex[(c >> 4) & 0x0F];
    out += hex[c & 0x0F];
  }

  return out;
}

static bool rakWaitForBoot(uint32_t timeoutMs) {
  uint32_t start = millis();

  while (millis() - start < timeoutMs) {
    if (sendAT("AT", "OK", 1000)) {
      return true;
    }

    delay(500);
  }

  return false;
}

bool initLoRa() {
  loraReady = false;
  loraState = "AT";
  txState = "NONE";
  updateTft();

  Serial.println();
  Serial.println("================================");
  Serial.println("RAK LORA INIT START");
  Serial.println("================================");

  bool atOk = false;

  for (int i = 0; i < 6; i++) {
    if (sendAT("AT", "OK", 1000)) {
      atOk = true;
      break;
    }

    delay(500);
  }

  if (!atOk) {
    loraReady = false;
    loraState = "NO AT";
    txState = "NO LORA";
    updateTft();

    Serial.println("LORA INIT FAILED: NO AT RESPONSE");
    return false;
  }

  sendAT("AT+VER=?", "OK", 1200);
  delay(150);

  sendAT("AT+NWM=?", "OK", 1200);
  delay(150);

  loraState = "NWM";
  updateTft();

  bool nwmOk = sendAT("AT+NWM=0", "OK", 2000);
  delay(500);

  if (!nwmOk) {
    Serial.println("WARNING: AT+NWM=0 did not return OK.");
  }

  Serial.println("RAK RESET AFTER NWM CHANGE...");
  sendAT("ATZ", "OK", 1000);

  delay(3500);

  loraState = "BOOT";
  updateTft();

  atOk = rakWaitForBoot(8000);

  if (!atOk) {
    loraReady = false;
    loraState = "NO AT";
    txState = "NO LORA";
    updateTft();

    Serial.println("LORA INIT FAILED: NO AT AFTER ATZ");
    return false;
  }

  loraState = "P2P";
  updateTft();

  String p2p = "AT+P2P=";
  p2p += String(LORA_FREQ_HZ);
  p2p += ":";
  p2p += String(LORA_SF);
  p2p += ":";
  p2p += String(LORA_BW);
  p2p += ":";
  p2p += String(LORA_CR);
  p2p += ":";
  p2p += String(LORA_PREAMBLE);
  p2p += ":";
  p2p += String(LORA_TX_POWER);

  bool p2pOk = sendAT(p2p, "OK", 3000);
  delay(500);

  sendAT("AT+PRECV=0", "OK", 1500);
  delay(300);

  if (!p2pOk) {
    Serial.println("WARNING: AT+P2P returned error.");
    Serial.println("Using previous RAK P2P config and allowing TX anyway.");

    loraReady = true;
    loraState = "READY";
    txState = "P2P OLD";
    updateTft();

    Serial.println("LORA INIT PARTIAL: READY WITH OLD P2P CONFIG");
    return true;
  }

  loraReady = true;
  loraState = "READY";
  txState = "READY";
  updateTft();

  Serial.println("LORA INIT OK: READY");
  return true;
}

String buildPacket() {
  gpsState = getGpsState();

  String p;
  p.reserve(96);

  p += DEVICE_ID;
  p += ",";
  p += gpsState;
  p += ",";

  if (hasGpsFix()) {
    p += compactLatLonPacketValue(gps.location.lat());
    p += ",";
    p += compactLatLonPacketValue(gps.location.lng());
    p += ",";
    p += String(safeSpeedKmh());
    p += ",";
    p += String(safeSat());
    p += ",";
    p += String(safeAlt());
    p += ",";
    p += String(activeSatCountFromGsv());
    p += ",";
    p += String(txCounter);
    p += ",";
    p += modeState;
  } else {
    p += "0,0,0,";
    p += String(safeSat());
    p += ",0,";
    p += String(activeSatCountFromGsv());
    p += ",";
    p += String(txCounter);
    p += ",";
    p += modeState;
  }

  return p;
}

bool sendLoRaPacket(const String &packet) {
  if (!loraReady) {
    txState = "NO LORA";
    updateTft();
    return false;
  }

  String hexPayload = textToHex(packet);
  String cmd = "AT+PSEND=" + hexPayload;

  bool ok = sendAT(cmd, "OK", 5000);

  if (ok) {
    txState = "OK #" + String(txCounter);
    lastPacket = packet;

    Serial.print("TX: ");
    Serial.println(packet);

    String evt = readRakResponse(2500);

    if (evt.length() > 0) {
      Serial.print("RAK: ");
      Serial.println(evt);
    }

    updateTft();
    return true;
  }

  txState = "FAIL";
  updateTft();

  Serial.print("TX FAIL: ");
  Serial.println(packet);

  return false;
}
