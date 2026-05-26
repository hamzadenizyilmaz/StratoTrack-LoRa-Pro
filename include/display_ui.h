#pragma once
#include <Arduino.h>

void beginDisplay();
void drawBootTft(const String &line1, const String &line2);
void drawStaticTft();
void updateTft();
