#ifndef TIME_SYNC_H
#define TIME_SYNC_H

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

void timeSync(String ssid, String password, bool forcePortal = false);

#endif