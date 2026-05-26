#include <Arduino.h>
#include "config.h"
#include "globals.h"
#include "gps_tracker.h"
#include "lora_modem.h"
#include "display_ui.h"
#include "web_panel.h"

void setup() {
  Serial.begin(DEBUG_SERIAL_BAUD);
  delay(2500);

  Serial.println();
  Serial.println("================================");
  Serial.println(FIRMWARE_NAME);
  Serial.print("Version: ");
  Serial.println(FIRMWARE_VERSION);
  Serial.println("Modular PRO build");
  Serial.println("================================");

  beginDisplay();
  drawBootTft("SYSTEM ONLINE", "GNSS + LoRa + TFT + Web Panel");
  delay(800);
  drawStaticTft();
  updateTft();

  beginGps();
  beginLoRaUart();

#if ENABLE_WEB_PANEL
  startWebPanel();
#endif

  delay(800);
  initLoRa();

  lastScreenMs = millis();
  lastTxMs = millis();
}

void loop() {
  readGps();

#if ENABLE_WEB_PANEL
  handleWebPanel();
#endif

  if (millis() - lastScreenMs >= SCREEN_INTERVAL_MS) {
    lastScreenMs = millis();

    updateTft();

    Serial.print("GPS ");
    Serial.print(getGpsState());
    Serial.print(" | SAT ");
    Serial.print(safeSat());
    Serial.print(" | GSV ");
    Serial.print(activeSatCountFromGsv());
    Serial.print(" | LAT ");
    Serial.print(latText());
    Serial.print(" | LON ");
    Serial.print(lonText());
    Serial.print(" | NMEA ");
    Serial.print(nmeaChars);
    Serial.print(" | LORA ");
    Serial.print(loraState);
    Serial.print(" | TX ");
    Serial.println(txState);
  }

  if (millis() - lastTxMs >= BEACON_INTERVAL_NORMAL_MS) {
    lastTxMs = millis();

    String packet = buildPacket();
    sendLoRaPacket(packet);

    txCounter++;
    updateTft();
  }
}
