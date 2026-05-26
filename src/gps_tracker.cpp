#include "gps_tracker.h"
#include <TinyGPSPlus.h>
#include "config.h"
#include "globals.h"

SatInfo sats[MAX_SATS];
int totalSatsInView = 0;

static int splitNmea(String line, String parts[], int maxParts) {
  int count = 0;
  int start = 0;

  int star = line.indexOf('*');
  if (star >= 0) {
    line = line.substring(0, star);
  }

  while (count < maxParts) {
    int comma = line.indexOf(',', start);

    if (comma < 0) {
      parts[count++] = line.substring(start);
      break;
    }

    parts[count++] = line.substring(start, comma);
    start = comma + 1;
  }

  return count;
}

static void clearSatList() {
  for (int i = 0; i < MAX_SATS; i++) {
    sats[i].prn = 0;
    sats[i].elev = -1;
    sats[i].az = -1;
    sats[i].snr = -1;
    sats[i].seenMs = 0;
  }
}

static void upsertSat(int prn, int elev, int az, int snr) {
  if (prn <= 0) return;

  int emptyIdx = -1;

  for (int i = 0; i < MAX_SATS; i++) {
    if (sats[i].prn == prn) {
      sats[i].elev = elev;
      sats[i].az = az;
      sats[i].snr = snr;
      sats[i].seenMs = millis();
      return;
    }

    if (sats[i].prn == 0 && emptyIdx < 0) {
      emptyIdx = i;
    }
  }

  if (emptyIdx >= 0) {
    sats[emptyIdx].prn = prn;
    sats[emptyIdx].elev = elev;
    sats[emptyIdx].az = az;
    sats[emptyIdx].snr = snr;
    sats[emptyIdx].seenMs = millis();
  }
}

static void parseGsvLine(String line) {
  if (!(line.startsWith("$GPGSV") || line.startsWith("$GNGSV"))) {
    return;
  }

  String p[32];
  int count = splitNmea(line, p, 32);

  if (count < 4) return;

  int sentenceNo = p[2].toInt();
  totalSatsInView = p[3].toInt();

  if (sentenceNo == 1) {
    clearSatList();
  }

  for (int idx = 4; idx + 3 < count; idx += 4) {
    int prn = p[idx].toInt();
    int elev = p[idx + 1].length() ? p[idx + 1].toInt() : -1;
    int az = p[idx + 2].length() ? p[idx + 2].toInt() : -1;
    int snr = p[idx + 3].length() ? p[idx + 3].toInt() : -1;

    if (prn > 0) {
      upsertSat(prn, elev, az, snr);
    }
  }
}

void beginGps() {
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS UART OK");
}

void readGps() {
  while (GPSSerial.available()) {
    char c = (char)GPSSerial.read();

    gps.encode(c);
    nmeaChars++;
    lastGpsCharMs = millis();

    if (c == '\n') {
      currentNmeaLine.trim();

      if (currentNmeaLine.length() > 6) {
        parseGsvLine(currentNmeaLine);
      }

      currentNmeaLine = "";
    } else if (c != '\r') {
      if (currentNmeaLine.length() < 120) {
        currentNmeaLine += c;
      } else {
        currentNmeaLine = "";
      }
    }
  }
}

bool hasGpsData() {
  return nmeaChars > 0 && (millis() - lastGpsCharMs < 5000);
}

bool hasGpsFix() {
  return gps.location.isValid() && gps.location.age() < 5000;
}

String getGpsState() {
  if (hasGpsFix()) return "FIX";
  if (hasGpsData()) return "DATA";
  return "WAIT";
}

int safeSat() {
  if (!gps.satellites.isValid()) return 0;

  int sat = gps.satellites.value();

  if (sat < 0 || sat > 99) return 0;

  return sat;
}

int safeSpeedKmh() {
  if (!gps.speed.isValid()) return 0;

  double v = gps.speed.kmph();

  if (v < 0 || v > 300) return 0;

  return (int)round(v);
}

int safeAlt() {
  if (!gps.altitude.isValid()) return 0;

  int alt = (int)round(gps.altitude.meters());

  if (alt < -500 || alt > 9000) return 0;

  return alt;
}

String latText() {
  if (!hasGpsFix()) return "---.------";
  return String(gps.location.lat(), 6);
}

String lonText() {
  if (!hasGpsFix()) return "---.------";
  return String(gps.location.lng(), 6);
}

String compactLatLonPacketValue(double value) {
  long v = (long)round(value * 1000000.0);
  return String(v);
}

int activeSatCountFromGsv() {
  int c = 0;
  unsigned long now = millis();

  for (int i = 0; i < MAX_SATS; i++) {
    if (sats[i].prn > 0 && now - sats[i].seenMs < 15000) {
      c++;
    }
  }

  return c;
}

String bestSatName() {
  int bestIdx = -1;
  int bestSnr = -1;

  for (int i = 0; i < MAX_SATS; i++) {
    if (sats[i].prn > 0 && millis() - sats[i].seenMs < 15000) {
      if (sats[i].snr > bestSnr) {
        bestSnr = sats[i].snr;
        bestIdx = i;
      }
    }
  }

  if (bestIdx < 0) return "-";

  String name = "G";
  if (sats[bestIdx].prn < 10) name += "0";
  name += String(sats[bestIdx].prn);
  name += " ";

  if (sats[bestIdx].snr < 0) name += "--";
  else name += String(sats[bestIdx].snr);

  name += "dB";

  return name;
}
