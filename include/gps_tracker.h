#pragma once
#include <Arduino.h>

struct SatInfo {
  int prn = 0;
  int elev = -1;
  int az = -1;
  int snr = -1;
  unsigned long seenMs = 0;
};

#define MAX_SATS 16

extern SatInfo sats[MAX_SATS];
extern int totalSatsInView;

void beginGps();
void readGps();

bool hasGpsData();
bool hasGpsFix();
String getGpsState();

int safeSat();
int safeSpeedKmh();
int safeAlt();

String latText();
String lonText();
String compactLatLonPacketValue(double value);

int activeSatCountFromGsv();
String bestSatName();
