#include "display_ui.h"

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include "config.h"
#include "globals.h"
#include "gps_tracker.h"

static Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);

#define C_BG        ILI9341_BLACK
#define C_HEADER    0x020F
#define C_PANEL     0x0861
#define C_PANEL2    0x10A2
#define C_LINE      0x33AF
#define C_WHITE     ILI9341_WHITE
#define C_GREY      ILI9341_DARKGREY
#define C_GREEN     ILI9341_GREEN
#define C_RED       ILI9341_RED
#define C_YELLOW    ILI9341_YELLOW
#define C_CYAN      ILI9341_CYAN
#define C_ORANGE    0xFD20
#define C_MAGENTA   0xF81F

static uint16_t gpsColor(const String &s) {
  if (s == "FIX") return C_GREEN;
  if (s == "DATA") return C_YELLOW;
  return C_RED;
}

static uint16_t loraColor(const String &s) {
  if (s == "READY") return C_GREEN;
  if (s == "P2P" || s == "AT" || s == "NWM" || s == "BOOT") return C_YELLOW;
  return C_RED;
}

static uint16_t snrColor(int snr) {
  if (snr >= 30) return C_GREEN;
  if (snr >= 15) return C_YELLOW;
  if (snr >= 1) return C_ORANGE;
  return C_GREY;
}

static void panel(int x, int y, int w, int h, uint16_t border, uint16_t fill) {
  tft.fillRoundRect(x, y, w, h, 8, fill);
  tft.drawRoundRect(x, y, w, h, 8, border);
}

static void label(int x, int y, const String &s) {
  tft.setTextSize(1);
  tft.setTextColor(C_GREY, C_PANEL);
  tft.setCursor(x, y);
  tft.print(s);
}

static void value(int x, int y, const String &s, uint16_t color, uint8_t size = 2) {
  tft.setTextSize(size);
  tft.setTextColor(color, C_PANEL);
  tft.setCursor(x, y);
  tft.print(s);
}

static void header() {
  tft.fillRect(0, 0, 320, 30, C_HEADER);
  tft.drawFastHLine(0, 30, 320, C_CYAN);

  tft.setTextSize(2);
  tft.setTextColor(C_WHITE, C_HEADER);
  tft.setCursor(9, 7);
  tft.print("STRATOTRACK");

  tft.setTextSize(1);
  tft.setTextColor(C_CYAN, C_HEADER);
  tft.setCursor(213, 7);
  tft.print("LoRa PRO");

  tft.setTextColor(C_YELLOW, C_HEADER);
  tft.setCursor(278, 7);
  tft.print(DEVICE_ID);
}

void beginDisplay() {
  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);

  tft.begin();
  tft.setRotation(DISPLAY_ROTATION);

#if TFT_INVERT_COLORS
  tft.invertDisplay(true);
#endif

  drawStaticTft();
}

void drawBootTft(const String &line1, const String &line2) {
  tft.fillScreen(C_BG);
  header();

  panel(18, 62, 284, 106, C_CYAN, C_PANEL);

  tft.setTextSize(2);
  tft.setTextColor(C_GREEN, C_PANEL);
  tft.setCursor(38, 90);
  tft.print(line1);

  tft.setTextSize(1);
  tft.setTextColor(C_GREY, C_PANEL);
  tft.setCursor(38, 126);
  tft.print(line2);

  tft.setTextColor(C_CYAN, C_BG);
  tft.setCursor(50, 195);
  tft.print("Offline GPS + Raw LoRa P2P Tracker");
}

void drawStaticTft() {
  tft.fillScreen(C_BG);
  header();

  panel(6, 38, 308, 76, C_CYAN, C_PANEL);
  panel(6, 120, 150, 52, C_GREEN, C_PANEL);
  panel(164, 120, 150, 52, C_MAGENTA, C_PANEL);
  panel(6, 178, 308, 56, C_ORANGE, C_PANEL);

  label(16, 46, "GNSS");
  label(78, 46, "USED");
  label(128, 46, "VIEW");
  label(180, 46, "BEST");
  label(16, 84, "LAT");
  label(165, 84, "LON");

  label(16, 128, "LORA RADIO");
  label(16, 151, "TX STATE");

  label(174, 128, "NMEA STREAM");
  label(174, 151, "COUNTER");

  label(16, 186, "SATELLITES  PRN:SNR");
}

void updateTft() {
  gpsState = getGpsState();

  tft.fillRoundRect(9, 41, 302, 70, 7, C_PANEL);

  label(16, 46, "GNSS");
  label(78, 46, "USED");
  label(128, 46, "VIEW");
  label(180, 46, "BEST");

  value(16, 59, gpsState, gpsColor(gpsState), 2);
  value(78, 59, String(safeSat()), C_WHITE, 2);
  value(128, 59, String(activeSatCountFromGsv()), C_CYAN, 2);
  value(180, 59, bestSatName(), C_YELLOW, 1);

  label(16, 84, "LAT");
  tft.setTextSize(1);
  tft.setTextColor(hasGpsFix() ? C_WHITE : C_GREY, C_PANEL);
  tft.setCursor(44, 84);
  tft.print(latText());

  label(165, 84, "LON");
  tft.setTextColor(hasGpsFix() ? C_WHITE : C_GREY, C_PANEL);
  tft.setCursor(193, 84);
  tft.print(lonText());

  tft.fillRoundRect(9, 123, 144, 46, 7, C_PANEL);
  label(16, 128, "LORA RADIO");
  value(16, 141, loraState.substring(0, min((int)loraState.length(), 8)), loraColor(loraState), 2);

  tft.setTextSize(1);
  tft.setTextColor(C_GREY, C_PANEL);
  tft.setCursor(16, 158);
  tft.print("TX ");
  tft.setTextColor(C_CYAN, C_PANEL);
  tft.print(txState.substring(0, min((int)txState.length(), 12)));

  tft.fillRoundRect(167, 123, 144, 46, 7, C_PANEL);
  label(174, 128, "NMEA STREAM");

  String nmeaState = hasGpsData() ? "LIVE" : "NO DATA";
  value(174, 141, nmeaState, hasGpsData() ? C_GREEN : C_RED, 2);

  tft.setTextSize(1);
  tft.setTextColor(C_GREY, C_PANEL);
  tft.setCursor(174, 158);
  tft.print("COUNT ");
  tft.setTextColor(C_CYAN, C_PANEL);
  tft.print(nmeaChars);

  tft.fillRoundRect(9, 181, 302, 50, 7, C_PANEL);
  label(16, 186, "SATELLITES  PRN:SNR");

  int x = 16;
  int y = 204;
  int printed = 0;

#if UI_SHOW_SATELLITES
  for (int i = 0; i < MAX_SATS && printed < 10; i++) {
    if (sats[i].prn > 0 && millis() - sats[i].seenMs < 15000) {
      tft.setTextSize(1);
      tft.setTextColor(snrColor(sats[i].snr), C_PANEL);
      tft.setCursor(x, y);

      tft.print("G");
      if (sats[i].prn < 10) tft.print("0");
      tft.print(sats[i].prn);
      tft.print(":");

      if (sats[i].snr < 0) tft.print("--");
      else tft.print(sats[i].snr);

      x += 55;
      printed++;

      if (printed == 5) {
        x = 16;
        y += 13;
      }
    }
  }
#endif

  if (printed == 0) {
    tft.setTextSize(1);
    tft.setTextColor(C_GREY, C_PANEL);
    tft.setCursor(16, 207);
    tft.print("Waiting for GSV satellite details...");
  }

#if UI_SHOW_WEB_INFO
  tft.fillRect(0, 235, 320, 5, C_BG);
#endif
}
